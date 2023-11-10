
# Purpose
Generates changelogs and credits for AgentMindStorm-style resource packs. Creates en_US and settings JSON UI files whose contents can be pasted directly into existing files.

This program MUST be run from the command line with 2 arguments!

# Usage

    ./ChangelogFormatter <changelog_input.txt> <pack_namespace>

    ./CreditsFormatter <credits_input.txt> <pack_namespace>

The changelog_input.txt file should be one full update worth of Changelog, from the update title to the end of the Notes section. Exclude `==============` dividers. This file must be in the same folder as the program.

The credits_input.txt file should be everything from the first to last contributor credit. Exclude anything past the `==============` dividers. This file must be in the same folder as the program.

The pack_namespace is arbitrary, but should match the pack the changelog is for. For example, Java Aspects uses pack_namespace `java_aspects`.

Output files are `en_US_[changelog/credits].lang` and `pack_[changelog/credits]_section.json`. These names do not match the default resources so you must copy their contents into the original resource pack files.
