#!/usr/bin/env python3
"""Regression checks for broken files, renamed headings, and Markdown link syntax."""

from pathlib import Path
import tempfile
import unittest

from check_doc_links import check_links, heading_anchors, markdown_links


class LinksTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        (self.root / 'docs').mkdir()
        (self.root / 'README.md').write_text('# Home\n')

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def check(self, text):
        self.write('docs/start.md', text)
        return check_links(self.root, ['docs/start.md'])

    def test_plain_bracketed_text(self):
        self.assertEqual(self.check('Rules marked **[tool]** are checked.'), [])

    def test_shortcut_reference_definition_rejected(self):
        self.assertIn('unsupported Markdown link', self.check('[home]\n\n[home]: ../README.md')[0])

    def test_relative_and_root_paths(self):
        self.assertEqual(self.check('[home](../README.md#home) [root](/README.md#home)'), [])

    def test_missing_file(self):
        self.assertIn('missing linked file', self.check('[broken](missing.md)')[0])

    def test_missing_heading_after_rename(self):
        self.write('docs/target.md', '# New title\n')
        self.assertIn('missing linked heading', self.check('[old](target.md#old-title)')[0])

    def test_fence_examples_and_inline_code(self):
        self.assertEqual(self.check('```md\n[bad](missing.md)\n```\n`[bad](other.md)`\n'), [])

    def test_nested_fence_delimiters(self):
        self.assertEqual(self.check('````md\n```\n[bad](missing.md)\n```\n````\n'), [])

    def test_unclosed_destination(self):
        self.assertIn('unsupported Markdown link', self.check('[bad](missing.md')[0])

    def test_external_links_not_fetched(self):
        self.assertEqual(self.check('[web](https://example.org/#a) [mail](mailto:x@example.org)'), [])

    def test_query_and_escaped_space(self):
        self.write('docs/a b.md', '# A title\n')
        self.assertEqual(self.check('[target](a%20b.md?raw=1#a-title)'), [])

    def test_unsupported_angle_destination_and_title(self):
        self.write('docs/a b.md', '# Target\n')
        self.assertIn('unsupported Markdown link', self.check('[target](<a b.md> "title")')[0])

    def test_balanced_parentheses(self):
        self.write('docs/a(b).md', '# Target\n')
        self.assertEqual(self.check('[target](a(b).md#target)'), [])

    def test_reference_and_collapsed_links_rejected(self):
        self.assertIn('unsupported Markdown link', self.check('[home][ref] [ref][]\n\n[ref]: ../README.md#home')[0])

    def test_missing_reference(self):
        self.assertIn('unsupported Markdown link', self.check('[home][no-definition]')[0])

    def test_image(self):
        self.assertIn('missing linked file', self.check('![image](absent.png)')[0])

    def test_escape_outside_repository(self):
        self.assertIn('leaves repository', self.check('[outside](../../outside.md)')[0])

    def test_duplicate_heading_collision(self):
        self.assertEqual(heading_anchors('# Topic\n# Topic\n# Topic-1\n'), {'topic', 'topic-1', 'topic-1-1'})

    def test_atx_markup_and_unicode_only(self):
        self.assertEqual(heading_anchors('# `SomeType` & **C++**\n## Café Θ\nSetext\n---\n'),
                         {'sometype--c', 'café-θ'})

    def test_fenced_heading_is_not_a_target(self):
        self.assertEqual(heading_anchors('```\n# Example\n```\n# Actual\n'), {'actual'})

    def test_explicit_anchor_is_not_a_heading(self):
        self.assertEqual(heading_anchors('<a name="custom"></a>\n# Header\n'), {'header'})

    def test_code_heading_preserves_underscores_and_angle_contents(self):
        self.assertEqual(heading_anchors('# `avela_radio` and `Template<T>`\n'),
                         {'avela_radio-and-templatet'})

    def test_atx_closing_hashes_and_italics(self):
        self.assertEqual(heading_anchors('## A _small_ **title** ##\n'), {'a-small-title'})


if __name__ == '__main__':
    unittest.main()
