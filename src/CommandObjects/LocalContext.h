//
// Created by dtcimbal on 15/07/2025.
#pragma once

#include <memory>
#include "CommandContext.h"

class LocalContext {
  public:
    LocalContext(std::unique_ptr<CommandContext>& Context) : mContext(Context) {
        if (mContext != nullptr) {
            mContext->Reset();
        }
    };
    ~LocalContext() = default;

    // Overload operator->
    CommandContext* operator->() const {
        if (mContext != nullptr) {
            return mContext.get();
        }
        return nullptr;
    }

    // Overload operator bool
    explicit operator bool() const {
        return mContext != nullptr;
    }

  private:
    std::unique_ptr<CommandContext>& mContext;
};
