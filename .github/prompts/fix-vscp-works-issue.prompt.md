---
description: "Use this prompt when you need to investigate, fix, and verify a bug or regression in the VSCP Works Qt/C++ codebase."
mode: agent
---

You are working in the VSCP Works repository and should treat this as a focused debugging-and-fix task.

Follow these steps:

1. Understand the problem
- Read the issue description or the relevant code path carefully.
- Identify the affected subsystem, such as UI dialogs, network/client code, or vendored VSCP protocol code.
- Prefer the smallest change that addresses the root cause.

2. Inspect the relevant code
- Check the existing implementation in the relevant source files under src/ and third_party/vscp/src/.
- Follow the repository conventions in AGENTS.md and .github/copilot-instructions.md.
- When changing a Qt dialog or widget, update the matching .ui, .cpp, and .h files together.

3. Diagnose before editing
- Reproduce the issue if possible.
- Trace the data flow and identify the underlying cause before making changes.
- Avoid speculative fixes or unrelated refactors.

4. Implement a targeted fix
- Keep changes compatibility-focused and aligned with the surrounding style.
- Avoid introducing new dependencies unless they are already used by the project.
- Prefer minimal, well-scoped edits over broad rewrites.

5. Verify the result
- Rebuild the project from the repository root with:
  - cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
  - cmake --build build --parallel
- If the task involves a UI change, check that the relevant dialog/widget still builds and behaves sensibly.
- Report the root cause, the change made, and the validation result.

When you respond, include:
- A short summary of the root cause.
- The files changed and why.
- The build verification outcome.
- Any follow-up suggestions if the issue may require further testing.
