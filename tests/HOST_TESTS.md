# Host tests

These checks use Python 3 and a native C++17 compiler (`g++`):

```sh
python tests/verify_packet.py
python tests/run_urgent_timing.py
g++ -std=c++17 -Wall -Wextra -Werror -I . -I src -I src/components/roost tests/roost_core_checks.cpp -o /tmp/roost_core && /tmp/roost_core
g++ -std=c++17 -Wall -Wextra -Werror -I . -I src -I src/components/roost tests/roost_custom_editor_checks.cpp -o /tmp/roost_editor && /tmp/roost_editor
```

The timing tests use modelled event injection and minimal platform stubs; they do not replace a firmware build or physical watch checks.
