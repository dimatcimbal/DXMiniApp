// src/Window/BaseView.h
#pragma once

#include <Windows.h>

// A base class for all UI view components.
// It manages the HWND for the specific view.
class BaseView {
  public:
    BaseView() : m_hParent(nullptr), m_hWnd(nullptr) {
    }
    // Virtual destructor is important for polymorphism.
    // No need to call DestroyWindow here, as Windows automatically destroys
    // child windows when their parent is destroyed.
    virtual ~BaseView() = default;

    // Returns the HWND of this view component.
    HWND GetHWND() const {
        return m_hWnd;
    }

    // hParent: The handle of the parent window.
    // id: A unique identifier for the child window.
    bool Create(HWND hParent, UINT id) {
        m_hParent = hParent;
        if (!OnCreate(hParent, id)) {
            m_hWnd = nullptr;
            return false;
        }
        return true;
    }

    // Virtual method to handle resizing of the view component.
    // Derived classes can override this for more complex internal layouts.
    virtual void OnSize(int x, int y, int width, int height) {
        if (m_hWnd) {
            MoveWindow(m_hWnd, x, y, width, height, TRUE);
        }
    }

  private:
    // Pure virtual method to create the view's specific window.
    // This must be implemented by derived classes.
    // hParent: The handle of the parent window.
    // id: A unique identifier for the child window.
    virtual bool OnCreate(HWND hParent, UINT id) = 0;

  protected:
    HWND m_hParent;
    HWND m_hWnd; // The HWND for this specific view component.
};