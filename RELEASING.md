# Releasing

How to cut a new GitHub release of Unfocused Hotkeys.

## 1. Prepare

- [ ] Bump `VersionName` (for example `1.1.0`) **and** `Version` (an integer that must increase every release) in `UnfocusedHotkeys.uplugin`.
- [ ] Move the `Unreleased` notes in `CHANGELOG.md` under the new version heading with today's date.
- [ ] The first time only: fill in `CreatedByURL`, `DocsURL` and `SupportURL` in the `.uplugin` with the GitHub repo and issues URLs.

## 2. Build

Run the packaging script once per engine version you want to support. Each engine must be installed locally.

```powershell
.\Scripts\Package.ps1 -EngineDir "C:\Program Files\Epic Games\UE_5.7", "C:\Program Files\Epic Games\UE_5.8"
```

This writes `dist/UnfocusedHotkeys-<version>-UE<engine>-Win64.zip` for each engine. Binaries from one engine minor version (5.8) work with all of its patch versions (5.8.x).

## 3. Smoke test

For each zip:

- [ ] Extract it into a **fresh** Blueprint-only project's `Plugins/` folder.
- [ ] Open the project. It should load without asking to rebuild.
- [ ] Build the Quick start graph from the README, press Play, alt-tab away, and press the hotkey.
- [ ] Package the project for Windows (Shipping) and run the packaged game once.

## 4. Publish

```powershell
git tag v1.1.0
git push origin v1.1.0
gh release create v1.1.0 dist/*.zip --title "v1.1.0" --notes-file <notes.md>
```

Use the matching `CHANGELOG.md` section as the release notes, and list the engine versions you built for.
