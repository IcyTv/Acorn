# Welcome to the Acorn Contributing Guide

Thank you for wanting to contribute to Acorn. This guide will help you get started.

When contributing, please keep in mind the GitHub [Code of Conduct](https://github.com/github/docs/blob/main/CODE_OF_CONDUCT.md).
Meaning: Be respectful and considerate of others.

In this guide you will get an overview of the contribution workflow from opening an issue, creating a PR, reviewing, and merging the PR.

## Getting Started

### Issues

#### Create an new issue

If you find an issue with our code, please do create an issue [here](https://github.com/IcyTv/Acorn/issues). Try and see if a related
issue exists. If you find one, but you think it doesn't have all the information, feel free to add to it!

#### Solve an issue

If you want to solve an issue, please do [create a pull request](#pull-requests). The branch format of your PR should be `issue/<issue-number>-<issue-description>`,
where the issue number is the #NO in the title of the issue.

If you can't create a pull request, do comment on the issue with your proposed solution.

### Pull Requests

#### Create a pull request

To create a pull request, please first [fork](https://github.com/IcyTv/Acorn/fork) the acorn repository.

Then [clone](https://git-scm.com/docs/git-clone) it to your local machine.

Create a new branch for your changes.

1. If you are fixing an issue, please use the format `issue/<issue-number>-<issue-description>`.
2. If you are adding a new feature, please use the format `feature/<feature-name>`.
3. For everything else, please use a descriptive name.

Perform your changes to this branch, and then push it to the remote repository.

Before creating a pull request, make sure the project still builds, even if you run `meson setup builddir` again!

Now you can create a pull request. Please use an existing template for your pull request and make sure to describe your changes well.

Don't forget to link the issue if you are solving one. Also enable the checkbox to allow for maintainer edits on your pull request.

Then wait for your pull request to be reviewed and merged 🎉🎉.

## Styles

If you are contributing, please make sure to adhere to our style conventions:

- Run clang-format on your code.
- Consider variable naming:
  - Use camelCase for variables.
  - Use m_PascalCase for class members and s_PascalCase for static members.
  - Make sure your variable names are descriptive. (eg. `m_I` is not descriptive, `m_CurrentIteratorPosition` is.)
- Class names should be PascalCase.
- Use `#pragma once` on header files.
- Only comment difficult code and consider if you can refactor it to not need commends.
- For classes and functions, write [doc comments](https://developer.lsst.io/cpp/api-docs.html).

## Commits

This repository uses [Atomic Commits](https://dev.to/paulinevos/atomic-commits-will-help-you-git-legit-35i7).
This means, you should write a lot of commits! The more, the merrier. Also explain a commit, if the change you do is not obvious.

For an example of how atomic commits, view <https://github.com/SerenityOS/serenity>.

If your commit is not atomic, please make sure to at least add a list of atomic changes (i. e. my commit does 1. add this class, 2. change this function, etc).

## CI

This repository uses Github Actions to run tests. If your pull request does not pass the tests, we cannot merge them into the master branch.
