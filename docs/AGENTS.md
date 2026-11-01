# Editing documentation

Read the [writing standard](WRITING_STANDARD.md) and the repository's [working instructions](../AGENTS.md). This
workflow applies to shared docs, component READMEs and docs, and documentation in agent instruction files.

After an edit:

1. Run `tools/check_docs.sh` and, for skin-facing references, `tools/check_skin_api.sh`.
2. Run the link/anchor check through `tools/check_docs.sh` and review code-cited headings.
3. Review the diff for retained facts, exact identifiers and values, mandatory rule strength, and correct ownership.
4. Show a readability rewrite as a diff to the person you work for before committing.

Follow the root instructions for the required build and code checks before a commit.
