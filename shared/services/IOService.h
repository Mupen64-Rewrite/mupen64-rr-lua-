/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

/*
 *	Interface for special IO-related functionality.
 *
 *	Must be implemented in the view layer.
 */

#include <string>
#include <vector>

namespace IOService
{
    /**
      * \brief Gets all files with a specific file extension directly under a directory
      * \param directory The directory
      * \param extension The file extension with no period
      */
    std::vector<std::wstring> get_files_with_extension_in_directory(std::wstring directory, const std::wstring& extension);
}
