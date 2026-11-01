#!/usr/bin/env python3
"""Check inline Markdown links and ATX heading anchors without dependencies.

Fenced examples and inline code are excluded. Reference links are rejected so a
new link cannot silently bypass validation. External URLs are not fetched.
"""

import html
from pathlib import Path
import re
import subprocess
import sys
import unicodedata
from urllib.parse import unquote, urlsplit


def prose_lines(text):
    fence = None
    for line_number, line in enumerate(text.splitlines(), 1):
        match = re.match(r'^\s*(`{3,}|~{3,})', line)
        if match:
            marker = match.group(1)
            if fence is None:
                fence = marker
            elif marker[0] == fence[0] and len(marker) >= len(fence) and not line.strip()[len(marker):].strip():
                fence = None
        elif fence is None:
            yield line_number, line


def heading_slug(text):
    code = []

    def keep_code(match):
        code.append(match.group(2))
        return f'ZZCODE{len(code) - 1}ZZ'

    text = re.sub(r'(`+)(.*?)\1', keep_code, text)
    text = re.sub(r'!?\[([^\]]*)\]\([^)]*\)', r'\1', text)
    text = re.sub(r'(?<!\w)_{1,2}(.+?)_{1,2}(?!\w)', r'\1', html.unescape(text))
    text = re.sub(r'[`*~]', '', text)
    for index, value in enumerate(code):
        text = text.replace(f'ZZCODE{index}ZZ', value)
    text = text.strip().lower()
    return ''.join('-' if character == ' ' else character for character in text
                   if character == ' ' or character in '-_' or unicodedata.category(character)[0] in 'LNM')


def heading_anchors(text):
    anchors = set()
    for _, line in prose_lines(text):
        match = re.match(r'^\s{0,3}#{1,6}\s+(.+?)(?:\s+#+\s*)?$', line)
        if match:
            slug = heading_slug(match.group(1))
            anchor, suffix = slug, 0
            while anchor in anchors:
                suffix += 1
                anchor = f'{slug}-{suffix}'
            anchors.add(anchor)
    return anchors


def markdown_links(text):
    for line_number, line in prose_lines(text):
        line = re.sub(r'(`+)(.*?)\1', lambda match: ' ' * len(match.group()), line)
        for match in re.finditer(r'!?\[[^\]\n]*\]', line):
            index = match.end()
            if index >= len(line) or line[index] != '(':
                if index < len(line) and line[index] in '[:':
                    yield line_number, None
                continue
            # Parentheses can occur inside a URL or a local filename.
            start, depth = index + 1, 1
            index = start
            while index < len(line) and depth:
                if line[index] == '\\':
                    index += 2
                    continue
                if line[index] == '(':
                    depth += 1
                elif line[index] == ')':
                    depth -= 1
                index += 1
            destination = line[start:index - 1]
            if depth or not destination or re.search(r'\s|[<>]', destination):
                yield line_number, None
            else:
                yield line_number, html.unescape(re.sub(r'\\(.)', r'\1', destination))


def check_links(root, documents):
    root = Path(root).resolve()
    errors, cache = [], {}
    for document in documents:
        path = root / document
        for line_number, href in markdown_links(path.read_text()):
            context = f'{document}:{line_number}'
            if href is None:
                errors.append(f'{context}: unsupported Markdown link; use [label](path) on one line')
                continue
            url = urlsplit(href)
            if url.scheme or url.netloc:
                continue
            target = (root / unquote(url.path).lstrip('/') if url.path.startswith('/') else
                      path.parent / unquote(url.path)) if url.path else path
            target = target.resolve()
            if not target.is_relative_to(root):
                errors.append(f'{context}: link leaves repository: {href}')
            elif not target.exists():
                errors.append(f'{context}: missing linked file: {href}')
            elif url.fragment and target.suffix.lower() == '.md':
                if target not in cache:
                    cache[target] = heading_anchors(target.read_text())
                if unquote(url.fragment) not in cache[target]:
                    errors.append(f'{context}: missing linked heading: {href}')
    return errors


def main():
    root = Path(__file__).resolve().parent.parent
    documents = sys.argv[1:] or subprocess.check_output(
        ['git', 'ls-files', 'README.md', 'AGENTS.md', 'docs/*.md',
         'components/*/AGENTS.md', 'components/*/README.md', 'components/*/docs/*.md'],
        cwd=root, text=True).splitlines()
    errors = check_links(root, documents)
    for error in errors:
        print(f'check_docs: {error}', file=sys.stderr)
    return bool(errors)


if __name__ == '__main__':
    raise SystemExit(main())
