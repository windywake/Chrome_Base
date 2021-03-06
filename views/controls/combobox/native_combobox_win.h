// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIEWS_CONTROLS_COMBOBOX_NATIVE_COMBOBOX_WIN_H_
#define VIEWS_CONTROLS_COMBOBOX_NATIVE_COMBOBOX_WIN_H_
#pragma once

#include "views/controls/combobox/native_combobox_wrapper.h"
#include "views/controls/native_control_win.h"

namespace views {

class NativeComboboxWin : public NativeControlWin,
                          public NativeComboboxWrapper {
 public:
  explicit NativeComboboxWin(Combobox* combobox);
  virtual ~NativeComboboxWin();

  // Overridden from NativeComboboxWrapper:
  virtual void UpdateFromModel() OVERRIDE;
  virtual void UpdateSelectedItem() OVERRIDE;
  virtual void UpdateEnabled() OVERRIDE;
  virtual int GetSelectedItem() const OVERRIDE;
  virtual bool IsDropdownOpen() const OVERRIDE;
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual View* GetView() OVERRIDE;
  virtual void SetFocus() OVERRIDE;
  virtual bool HandleKeyPressed(const views::KeyEvent& event) OVERRIDE;
  virtual bool HandleKeyReleased(const views::KeyEvent& event) OVERRIDE;
  virtual void HandleFocus() OVERRIDE;
  virtual void HandleBlur() OVERRIDE;
  virtual gfx::NativeView GetTestingHandle() const OVERRIDE;

 protected:
  // Overridden from NativeControlWin:
  virtual bool ProcessMessage(UINT message,
                              WPARAM w_param,
                              LPARAM l_param,
                              LRESULT* result) OVERRIDE;
  virtual void CreateNativeControl() OVERRIDE;
  virtual void NativeControlCreated(HWND native_control) OVERRIDE;

 private:
  void UpdateFont();

  // The combobox we are bound to.
  Combobox* combobox_;

  // The min width, in pixels, for the text content.
  int content_width_;

  DISALLOW_COPY_AND_ASSIGN(NativeComboboxWin);
};

}  // namespace views

#endif  // VIEWS_CONTROLS_COMBOBOX_NATIVE_COMBOBOX_WIN_H_
