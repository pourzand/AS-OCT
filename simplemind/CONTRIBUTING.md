# Contributing to SimpleMind

## Quick Start

SimpleMind (SM) uses `develop` as its primary branch.

To add a feature or fix a bug, please:

1. Create a new feature branch off of `develop`
2. Make your edits and changes
3. Commit your edits and changes to your branch
4. Push your branch to the SM repository
5. Open a merge request describing what your changes accomplish and why they might be important for SM

From the command line, this will look something like:

```
git checkout develop && git pull
git checkout -b <your-name-or-some-id>/my-cool-new-feature
<make your changes>
git add <your changed files>
git commit -m <a concise but descriptive commit message>
git push -u origin <your-name-or-some-id>/my-cool-new-feature
```

To open the merge request, either click the link provided in the shell window after you push your code, or navigate to GitHub and use the UI to open a merge request for your branch.

## Branch Management

Our two protected branches for SimpleMind are `main` and `develop`.  In general, both of these branches should be in reasonably healthy state, meaning code should build, pipelines should pass, etc. `develop` will occasionally break as major changes are made, or new features are added. `main` should never be broken.

In general, code should *never* be committed directly to `develop`.  It is ok for there to be exceptions to this sometimes, however it should be exceedingly rare.

Code should **never** be committed directly to `main`.  Commits should be added to main by opening a merge request to merge `develop` into `main`.  This will be handled by maintainers when it is determined that it is time for a new release, *after* the current develop state is deemed to be ready (i.e. tested).