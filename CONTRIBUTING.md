# Contributing to R-Type
This is written for anyone who wants to contribute to the R-Type repository.

## See also
* [`ARCHITECTURE.md`](ARCHITECTURE.md)

## What to contribute
* **Features**: We welcome any new features you would like to add.

* **Bug reports and issues**: Open them at <https://github.com/DiegoP-G/mirror_r-type/issues>.


## Pull requests

Open draft PR:s to get some early feedback on your work until you feel it is ready for a proper review.
Do not make PR:s from your own `main` branch, as that makes it difficult for reviewers to add their own fixes.
Add any improvements to the branch as new commits instead of rebasing to make it easier for reviewers to follow the progress (add images if possible!).

All PR:s are merged with [`Squash and Merge`](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/incorporating-changes-from-a-pull-request/about-pull-request-merges#squash-and-merge-your-commits), meaning they all get squashed to just one commit on the `main` branch. This means you don't need to keep a clean commit history on your feature branches. In fact, it is preferable to add new commits to a branch rather than rebasing or squashing. For one, it makes it easier to track progress on a branch, but rebasing and force-pushing also discourages collaboration on a branch.

## Code formatting
You must use [pre-commit](https://pre-commit.com/) to format your code before pushing.

After installing pre-commit, at the root of the repository, run :
```bash
pre-commit install
```

You can test it by running :
```bash
pre-commit run --all-files
```