# Releasing

Releases are automated with [release-please](https://github.com/googleapis/release-please).
You never tag by hand and you never edit the version number directly.

## How it works

1. **Write Conventional Commit messages.** The commit *type* decides the next version:

   | Commit | Effect (pre-1.0) |
   |---|---|
   | `fix: ...` | patch bump — `0.2.0` → `0.2.1` |
   | `feat: ...` | minor bump — `0.2.0` → `0.3.0` |
   | `feat!: ...` or a `BREAKING CHANGE:` footer | minor bump while `0.x` (this is what to use for the eventual `1.0.0` — see below) |
   | `docs:` `refactor:` `perf:` `build:` | shown in the changelog, no bump on their own |
   | `ci:` `chore:` `test:` | no bump, hidden from the changelog |

   Merge PRs as **merge commits** so release-please sees each message. If you squash-merge, the
   **PR title** must itself be a Conventional Commit.

2. **release-please keeps a "Release PR" open.** Every push to `main` updates a PR titled
   `chore(main): release X.Y.Z`. It bumps `version.txt`, the `project(VERSION ...)` line in
   `CMakeLists.txt`, and prepends the new section to `CHANGELOG.md`.

3. **Cut the release: merge that PR.** That is the only manual step. On merge, the `Release`
   workflow creates the `vX.Y.Z` tag and the GitHub Release, then builds the Linux artefacts
   (AppImage, `.deb`, `VST3/LV2/CLAP` tarball) and attaches them. The release shows up first
   with just the notes; the binaries land a few minutes later when the build finishes.

While the project is `0.x` every release is published as a GitHub **pre-release**.

## Going to 1.0.0

release-please will not bump `0.x` to `1.0.0` on its own. When you are ready, edit the open
Release PR's version (or its title) to `1.0.0` and merge, or land a commit with a
`Release-As: 1.0.0` footer. After that, `feat!:` / `BREAKING CHANGE:` bump the major as usual.

## Notes

- `ci.yml` does **not** run on the Release PR — GitHub does not trigger workflows for commits
  made by the built-in `GITHUB_TOKEN`. That PR only changes the version and changelog, so
  merge it once the normal CI on the feature PRs was green.
- The tag/version scheme and the workflow live in `release-please-config.json`,
  `.release-please-manifest.json` and `.github/workflows/release.yml`.

## If the build/publish step gets skipped

The `publish` job only runs when release-please reports `release_created == 'true'` on the push
that merges the Release PR. That output has intermittently come back empty (v0.5.1, v0.5.3; cause
unconfirmed — see `BUG-004` in `docs/backlog.md`) even though the tag and GitHub Release were
created fine. Two visible symptoms: the release has no `.deb`/AppImage/plugin tarball attached,
and it's stuck showing `prerelease: false` (only the `publish` step sets `prerelease: true` for
0.x), which can make an old release outrank a newer one for GitHub's "Latest" badge.

**After every merged Release PR, check the release's assets on GitHub** — don't assume
`publish` ran just because the merge was clean.

To backfill it: **Actions → Release → Run workflow**, and give it the existing tag (e.g.
`v0.5.1`) in the `tag` input. This runs the `publish` job alone against that tag, building from
`main`'s current tip (not the historical tag — see `BUG-004`/`BUG-005` in `docs/backlog.md`),
independent of release-please — safe to re-run any time, it rebuilds and re-attaches the
artefacts and fixes the prerelease flag on that release in place.
