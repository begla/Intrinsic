// Copyright 2016 Benjamin Glatzel
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

class IntrinsicEdNotificationBase : public QWidget
{
  Q_OBJECT

public:
  IntrinsicEdNotificationBase(QWidget* p_Parent, float p_TimeToLive);
  ~IntrinsicEdNotificationBase();

protected slots:
  void onTick();

protected:
  void positionOnScreen();

  QTimer* _timer;
  float _timeAliveInSeconds;
  float _timeToLive;
};
