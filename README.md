# Purpose

Generates changelogs and credits for AgentMindStorm-style resource packs. Creates en_US and settings JSON UI files whose contents can be pasted directly into existing files.

This program MUST be run from the command line with 2 arguments!

# Usage

    ./changelog_formatter.exe <changelog_input.txt> <pack_namespace>

    ./credits_formatter.exe <credits_input.txt> <pack_namespace>

The `changelog_input.txt file` should be one full update, from the title to the end of the Notes section. Exclude `==============` dividers. This file must be in the same folder as the executable.

The `credits_input.txt file` should be everything from the first to last contributor credit. Exclude the `==============` dividers and anything beyond. This file must be in the same folder as the executable.

Use the `social_media_colors.txt` file to configure a unique color for each social media platform. Arbitrary platform names are supported, so the program does not need to be recompiled to add new social media. The social media name to match is in the left column, while the formatting color to use is in the right column, separated by one space. Formatting colors **must** be one character.

The `pack_namespace` is arbitrary, but should match the pack the changelog is for. For example, Java Aspects uses pack_namespace `java_aspects`.

Output files are `en_US_[changelog/credits].lang` and `pack_[changelog/credits]_section.json`. These names do not match the default resources. You must copy their contents into the original resource pack files.

# License

Copyright AgentMindStorm. Some rights reserved. You are free to implement my style of custom tab into your own pack and use these tools to generate your files as long as you credit AgentMindStorm for this program. You are *not* permitted to modify or distribute this program.

# JSON Library

Uses the [nlohmann-json](https://github.com/nlohmann/json) header for JSON output.

The class is licensed under the MIT License:

Copyright © 2013-2022 Niels Lohmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
