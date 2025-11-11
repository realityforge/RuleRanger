# Repository Guidelines

## Authoritative Repository

The canonical source for this plugin is maintained at https://github.com/realityforge/RuleRanger. Please open pull
requests and issues on that repository.

This plugin is sometimes integrated into other repositories via git subtree merge or distributed as a direct download (
unzip into `Plugins/RuleRanger/`). Those copies are mirrors for consumption; treat them as downstream integrations and
avoid filing issues/PRs there.

## User Interaction

When asked to perform a task, ask the user questions one at a time until you have enough context. Feel free to make
reasonable assumptions based on patterns present in the code and ask the user to confirm the assumptions if there are
reasonable alternatives.

## Project Structure & Module Organization

- `RuleRanger.uplugin` declares modules; each module sits under `Source/<Module>/Public` and `Source/<Module>/Private`
  so shared headers stay isolated from implementation-only details. There exists a single `RuleRanger` module at this time.
- The `RuleRanger` plugin has several modules that are separated based on dependencies:
  - `Source/RuleRanger` - This contains the core RuleRanger engine and some common actions and matchers.
  - `Source/RuleRangerGameplayTags` - This contains matchers and actions that dependend on the GameplayTags module.
  - `Source/RuleRangerMetaSound` - This contains matchers and actions that dependend on the MetaSounds module.
  - `Source/RuleRangerNiagara` - This contains matchers and actions that dependend on the Niagara module.
- Matcher and action classes defined as a header and implementation pair present in `Source/[RuleRangerModule]/Private/RuleRanger/*` subdirectories.
- Raw files (such as `.csv` files) from which Unreal assets are imported belong in `SourceContent`, while `Content` is
reserved for runtime assets that ship with the plugin.
- Generated binaries and build artifacts should stay out of version control and should stay untouched unless you are
  troubleshooting a local build.
- Keep `README.md` aligned with new features so downstream teams stay informed.

## Tooling & Engine Version

- Target Unreal Engine 5.6 for both development and verification; earlier engine releases are unsupported.

## General Principles

- Readability: Write code that is easy to read and understand. Prioritize clarity over overly clever or obscure
  solutions.
- Consistency: Strive for consistency in naming, formatting, and architectural patterns throughout the project.
- Simplicity (KISS): Keep It Simple, Stupid. Avoid unnecessary complexity.
- Don't Repeat Yourself (DRY): Avoid code duplication. Utilize functions, classes, and reusable components.
- Commenting:
    - Comment code that is complex, non-obvious, or critical.
    - Explain why something is done, not just what is being done (if the what is clear from the code).
    - Keep comments up-to-date with code changes.
- Modularity: Design components to be as self-contained and reusable as possible.
- Performance: Be mindful of performance implications, especially for real-time operations. Profile and optimize
  critical code paths.
- Error Handling: Implement robust error handling and provide clear feedback to users or logs when errors occur.

## Coding Style & Naming Conventions

- Follow Unreal Engine defaults: 4-space indentation, PascalCase types, Functions and Variables, `F[StructName]` for
  structs, `U[ClassName]` for UObject classes, and `E[EnumName]` for enums, `b[BooleanVariable]`.
- Use type deduction (for example, `auto`) when it keeps intent clear and remains within Unreal Engine 5.6’s supported
  C++ feature set.
- Place new public headers under `Source/<Module>/Public/<Module>/` and implementation files under the matching
  `Private` path.
- Prefer UE logging macros with the `LogRuleRanger` category; declare new categories in module `Private` headers when
  needed.
- Keep includes ordered and minimal; prefer forward declarations in headers. Use #pragma once for include guards.
- Use Doxygen-style comments for documenting classes, functions, and significant variables for API documentation
  generation.
- Match existing code style when not covered by the above rules.

## Unreal Live Reload Notes

- Avoid defining helper functions as `static` inside anonymous namespaces in `.cpp` files. Unreal's Live Coding/Live Reload can load multiple copies of a module during iteration; anonymous-namespace `static` functions produce multiple distinct internal symbols on reload and can lead to ODR-like issues and confusing behavior.
- Prefer one of the following patterns instead:
  - Wrap helpers in an internal `namespace RuleRanger::Tool` (or similar) with non-`static` functions.
  - Use `static` member functions on a small internal class/struct in the same `.cpp` if encapsulation is needed.
  - Place shared helpers in a `.h`/`.cpp` pair with proper linkage and include where needed.
- When in doubt, favor explicit namespaces and normal linkage over anonymous namespaces for code that may be touched during Live Coding.

## Testing Guidelines

- Automation coverage is aspirational. Capture edge cases in unit-style specs once a testing harness lands under
  `Source/<Module>/Private/Tests/`.
- Until formal suites exist, document manual reproduction steps or sample maps in the pull request so reviewers can
  exercise the change.

## Commit & Pull Request Guidelines

- Write imperative, present-tense commit messages under 72 characters.
- Squash noisy work-in-progress commits locally; each change should stand on its own.
- Open pull requests with a clear summary, reproduction or test notes, and screenshots or GIFs when changes impact
  in-editor UX.
