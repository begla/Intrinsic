// Intrinsic
// Copyright (c) 2016 Benjamin Glatzel
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once

namespace Intrinsic
{
namespace Core
{
namespace Algorithm
{
template <class Type, class ComparatorType>
_INTR_INLINE void parallelSort(_INTR_ARRAY(Type) & p_Array,
                               const ComparatorType& p_Comparator)
{
  if (p_Array.empty())
  {
    return;
  }

  const uint32_t partitionCount =
      (Application::_scheduler.GetNumTaskThreads() *
       (Application::_scheduler.GetNumTaskThreads() - 1u));

  struct SortTaskSet : enki::ITaskSet
  {
    virtual ~SortTaskSet() {}

    void ExecuteRange(enki::TaskSetPartition p_Range,
                      uint32_t p_ThreadNum) override
    {
      _INTR_PROFILE_CPU("Algorithm", "Sort");

      std::sort(_array->begin() + _start, _array->begin() + _end, _comparator);
    };

    ComparatorType _comparator;
    _INTR_ARRAY(Type) * _array;
    uint32_t _start;
    uint32_t _end;
  };

  struct MergeTaskSet : enki::ITaskSet
  {
    virtual ~MergeTaskSet() {}

    void ExecuteRange(enki::TaskSetPartition p_Range,
                      uint32_t p_ThreadNum) override
    {
      _INTR_PROFILE_CPU("Algorithm", "Merge");

      std::inplace_merge(_array->begin() + _start, _array->begin() + _middle,
                         _array->begin() + _end, _comparator);
    };

    ComparatorType _comparator;
    _INTR_ARRAY(Type) * _array;
    uint32_t _start;
    uint32_t _middle;
    uint32_t _end;
  };

  _INTR_ARRAY(MergeTaskSet) mergeTaskSets;
  mergeTaskSets.resize(partitionCount);
  _INTR_ARRAY(SortTaskSet) sortTaskSets;
  sortTaskSets.resize(partitionCount);

  uint32_t actualPartCount = 0u;
  uint32_t elementsPerPart =
      (uint32_t)std::ceil(p_Array.size() / (float)partitionCount);

  // Parallel sort
  {
    uint32_t rangeLeft = (uint32_t)p_Array.size();
    uint32_t currentElementsPerPart = elementsPerPart;

    while (rangeLeft > 0)
    {
      if (currentElementsPerPart > rangeLeft)
      {
        currentElementsPerPart = rangeLeft;
      }

      SortTaskSet& taskSet = sortTaskSets[actualPartCount];
      taskSet._array = &p_Array;
      taskSet._comparator = p_Comparator;
      taskSet._start = (uint32_t)p_Array.size() - rangeLeft;
      taskSet._end = taskSet._start + currentElementsPerPart;

      Application::_scheduler.AddTaskSetToPipe(&taskSet);

      ++actualPartCount;
      rangeLeft -= currentElementsPerPart;
    }

    for (uint32_t sortTaskIdx = 0u; sortTaskIdx < actualPartCount;
         ++sortTaskIdx)
    {
      Application::_scheduler.WaitforTaskSet(&sortTaskSets[sortTaskIdx]);
    }
  }

  // Parallel merge
  {
    uint32_t mergeSize = 2u;

    while (true)
    {
      uint32_t mergeTaskCount = 0u;
      for (uint32_t mergePartIdx = 0u; mergePartIdx < actualPartCount;
           mergePartIdx += mergeSize)
      {
        MergeTaskSet& taskSet = mergeTaskSets[mergeTaskCount];
        taskSet._array = &p_Array;
        taskSet._comparator = p_Comparator;
        taskSet._start = mergePartIdx * elementsPerPart;
        taskSet._end = (mergePartIdx + mergeSize) * elementsPerPart;
        taskSet._middle = (taskSet._start + taskSet._end) / 2u;

        // Clamp the last partition
        taskSet._end = std::min(taskSet._end, (uint32_t)p_Array.size());
        taskSet._middle = std::min(taskSet._middle, (uint32_t)p_Array.size());

        if (taskSet._middle == taskSet._end)
        {
          break;
        }

        Application::_scheduler.AddTaskSetToPipe(&taskSet);
        ++mergeTaskCount;
      }

      if (mergeTaskCount == 0u)
      {
        break;
      }

      for (uint32_t mergeTaskIdx = 0u; mergeTaskIdx < mergeTaskCount;
           ++mergeTaskIdx)
      {
        Application::_scheduler.WaitforTaskSet(&mergeTaskSets[mergeTaskIdx]);
      }

      mergeSize *= 2u;
    }
  }
}
}
}
}
