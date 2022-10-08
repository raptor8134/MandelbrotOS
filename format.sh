#!/bin/bash
find ./src -type f \( -iname \*.h -o -iname \*.c \) -exec clang-format -i -style="{IndentWidth: 2,TabWidth: 2,IndentGotoLabels: true,IndentCaseLabels: true,KeepEmptyLinesAtTheStartOfBlocks: true,ConstructorInitializerIndentWidth: 2, ContinuationIndentWidth: 2,}" {} \;
