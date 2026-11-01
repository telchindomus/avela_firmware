# Writing standard

How Avela's docs are written and where they live: everything in `docs/`, each component's `README.md` and `docs/`,
and the `AGENTS.md` files. People and AI tools read the same text. It must be readable without losing technical
precision. Code comments follow `docs/developer/CODING_STANDARD.md`, section 9.

## Voice

- **Write for a person first.** Use whole sentences, plain words and the order a reader needs: what it is, why it is
  so, then the details. Make the text as readable as you can, as long as no precision is lost. When readability and
  precision pull apart, precision wins.
- **Say what is and why.** The docs describe the firmware as it is now and the reasons for its shape. They never say
  who decided something, when, where an idea came from, how it used to be, or what is planned. History lives in git.
- **State rules plainly.** "must", "never" and "only" mean exactly that. Don't turn a rule into a suggestion ("never
  `new`" doesn't become "try to avoid `new`"), and don't make a suggestion sound like a rule.

## Precision

- **Identifiers, file paths, numbers and units are exact**, character for character: `AvelaBoardConcept`,
  `docs/developer/BUILD.md`, `0x10000`, 20 ms, GPIO22. Code, file names and commands go in backticks.
- **References must resolve.** Name existing files and folders accurately. If a procedure creates a file, say so.
  `tools/check_docs.sh` checks path tokens, relative Markdown links, and heading anchors as part of
  `tools/check_code.sh`. Review context-dependent examples and technical claims too.
- **Change a linked heading only together with its links.** `tools/check_docs.sh` resolves relative Markdown links
  and their heading anchors. Use clickable inline links in the form `[label](path)` for repository navigation, and
  ATX headings beginning with `#`. Keep each link on one line; encode spaces in paths as `%20`. The gate rejects
  reference-style links and unsupported destinations.
- **Preserve code-cited headings and identifiers.** The check cannot discover every prose citation in source
  comments. Keep the numbered rules in SKIN_ARCHITECTURE, the numbered CODING_STANDARD sections, and SKIN_CHANGES
  version headings unless every consumer is updated. Before renaming a section named in code or instructions, find
  and update its citations with `git grep`.

## Where the docs live

The repository and each component are laid out the same way: working rules at the root, the docs in `docs/`, split
by who reads them.

```
docs/                          the repository's docs: README.md (which doc to read), WRITING_STANDARD.md, AGENTS.md
  developer/                   for people who change the firmware or write skins
  user/                        for people who use the radio
components/<component>/
  README.md                    brief: what the component is in a sentence or two, and links to its docs
  AGENTS.md                    only when the component has working rules of its own; CLAUDE.md beside it imports it
  docs/
    developer/
      <component>.md           how the component works and why
      SKIN.md                  for skin authors, when the component has page skins or shared skin pieces
    user/                      what a user sees and sets
```

- **A fact about one component lives in that component's docs.** The repository's docs hold what spans components:
  the architecture, the coding rules, the controls, skins in general, building and the checks.
- **A component's docs go with its folder**, so retiring the component deletes them too.
- **Nothing is created empty.** A folder or a page appears with its first content, and a component gets its
  `README.md` with its first doc. The README holds no details of its own; they belong in the docs it links to.

## One fact, one place

- Each fact lives in exactly one doc. Elsewhere, name that file and its section instead of repeating the text.
- The facts live in the docs (`docs/` and the components' `docs/`). An `AGENTS.md` holds working rules and points
  into the docs; it never repeats what a doc says, and a doc never repeats an `AGENTS.md`.

## Public by default

The repository is public, so the docs contain nothing private:

- no shop or seller names: parts are named by what they are (a CYD 1.9", a MAX98357A breakout), not where they were
  bought;
- no dates, quotes of decisions or plans;
- no references to private material: no private paths, and no names of design folders, mockups, notes or other files
  that aren't in the repository. A doc describes what the repository holds, and says it without them.

## Changing the docs

- **A rewrite adds no facts.** Rewrite what is there; don't add explanations, numbers or behaviour that neither the
  text nor the code gives. If something is unclear, ask instead of guessing.
- **A change to the code updates its doc** in the same commit (root `AGENTS.md`, "When you change the code, update
  the docs").
- **Review the diff before committing:** same facts, same rule strength, same cited headings. Follow the shared
  [editing workflow](AGENTS.md), including for component-local documentation.
