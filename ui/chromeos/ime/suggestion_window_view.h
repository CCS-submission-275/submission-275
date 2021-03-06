// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_CHROMEOS_IME_SUGGESTION_WINDOW_VIEW_H_
#define UI_CHROMEOS_IME_SUGGESTION_WINDOW_VIEW_H_

#include <memory>

#include "base/macros.h"
#include "ui/chromeos/ui_chromeos_export.h"
#include "ui/views/bubble/bubble_dialog_delegate_view.h"

namespace ui {
namespace ime {

class SuggestionView;

// SuggestionWindowView is the main container of the suggestion window UI.
class UI_CHROMEOS_EXPORT SuggestionWindowView
    : public views::BubbleDialogDelegateView {
 public:
  explicit SuggestionWindowView(
      gfx::NativeView parent,
      int window_shell_id =
          -1 /* equals ash::ShellWindowId::kShellWindowId_Invalid */);
  ~SuggestionWindowView() override;
  views::Widget* InitWidget();

  // Hides.
  void Hide();

  // Shows suggestion text.
  void Show(const base::string16& text);
  void UpdateSuggestion(const base::string16& text);

  void SetBounds(const gfx::Rect& cursor_bounds);

 private:
  friend class SuggestionWindowViewTest;

  // views::BubbleDialogDelegateView:
  const char* GetClassName() const override;

  // The suggestion view is used for rendering suggestion.
  SuggestionView* suggestion_view_;

  // Corresponds to ash::ShellWindowId.
  const int window_shell_id_;

  DISALLOW_COPY_AND_ASSIGN(SuggestionWindowView);
};

}  // namespace ime
}  // namespace ui

#endif  // UI_CHROMEOS_IME_SUGGESTION_WINDOW_VIEW_H_
