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

// UI related includes
#include "ui_IntrinsicEdNotificationSimple.h"

class IntrinsicEdNotificationSimple : public IntrinsicEdNotificationBase
{
  Q_OBJECT

public:
  IntrinsicEdNotificationSimple(QWidget* p_Parent, const char* p_Text,
                                float p_TimeToLive = 3.0f);
  ~IntrinsicEdNotificationSimple();

private:
  Ui::IntrinsicEdNotificationSimpleClass _ui;
};
