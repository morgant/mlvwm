# Contributing to MLVWM

We welcome feedback, bug fixes, and feature request. Ultimately, what gets accepted and merged into the project is up to the current project maintainer ([Morgan Aldridge](https://github.com/morgant)) and what they feel is appropriate for all users of the project, but you are encouraged to submit any suggestions.

Please submit bugs and feature requests via the project’s [issue tracker](https://github.com/morgant/mlvwm/issues).

## What You Need

You will need the following to contribute:

* A [GitHub](http://github.com) account for submitting pull requests
* X11

## Making Changes

Follow these steps when making changes. That way, they will most likely be accepted with few headaches and very little back and forth.

1. Fork the [mlvwm](https://github.com/morgant/mlvwm) project on GitHub.
2. Create a topic branch from the `master` branch. Name your branch appropriately, reflecting the intended changes (e.g. “sprintf-to-snprintf” or “scroll-bar-theme-improvements”)
3. Make your edits (please try to conform to our [style guide](#style-guide)).
4. Make commits in logical units and with concise but explanatory commit messages. Please reference any appropriate issue numbers, e.g. "Issue #16".
5. Ensure your changes build without additional warnings or errors. We suggest testing in `Xephyr`.

## Submitting Changes

When you’ve completed your changes and are ready to merge them into the main project, follow these steps to submit them for review.

1. Push the changes to your fork of the [mlvwm](https://github.com/morgant/mlvwm) project on GitHub
2. Submit a pull request to the [mlvwm](https://github.com/morgant/mlvwm) project

That’s all there is to it.

If you followed the [making changes](#making-changes) guidelines and the changes are aligned with the vision of the project, it should be a smooth process to merge them.

## Style Guide

This is a very old codebase from before syle guides were a thing and we haven't yet tackled standardizing the formatting. So, for the time being, please try to use the same style as surrounding code, including either spaces or tabs for indendation (whichever is used locally), spacing around parentheses, etc. Please remove any trailing whitespace from lines.
