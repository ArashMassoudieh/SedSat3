#!/usr/bin/env python3
"""Convert SedSAT3_code_review_response.tex to a Word document.

Only the constructs that letter uses are supported (its \\group, reviewer and
response environments, \\status, \\todo, \\mine, \\code, itemize, quote,
tabular/longtable, tcolorbox). The .docx is written directly as Office Open
XML with the Python standard library, so no converter needs installing.

    python3 tex2docx.py SedSAT3_code_review_response.tex SedSAT3_code_review_response.docx
"""
import datetime
import re
import sys
import zipfile
from xml.sax.saxutils import escape

RED, BLUE, GREEN = "BE1E1E", "1446AA", "00643C"

# ---------------------------------------------------------------- helpers

def matching_brace(s, i):
    """Index of the '}' matching the '{' at s[i]."""
    depth = 0
    while i < len(s):
        c = s[i]
        if c == "\\":
            i += 2
            continue
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise ValueError("unbalanced braces")


SUPERSCRIPT = str.maketrans("0123456789-+", "⁰¹²³⁴⁵⁶⁷⁸⁹⁻⁺")


def math_to_text(m):
    m = m.replace("\\lambda", "λ").replace("\\sigma", "σ").replace("\\delta", "δ")
    m = m.replace("\\infty", "∞").replace("\\log", "log").replace("\\,", " ")
    m = re.sub(r"\\mathrm\{([^}]*)\}", r"\1", m)
    m = re.sub(r"\^\{([^}]*)\}", lambda x: x.group(1).translate(SUPERSCRIPT), m)
    m = re.sub(r"\^(\w)", lambda x: x.group(1).translate(SUPERSCRIPT), m)
    m = m.replace("<", " < ").replace("  ", " ")
    return m.strip()


def plain(t):
    """Replace LaTeX character macros by Unicode."""
    t = t.replace("\\allowbreak{}", "").replace("\\allowbreak", "")
    t = t.replace("\\textbackslash{}", "\\")
    for a, b in [("\\_", "_"), ("\\#", "#"), ("\\%", "%"), ("\\&", "&"), ("\\{", "{"), ("\\}", "}"),
                 ("\\ldots", "…"), ("\\S", "§"), ("\\today", datetime.date.today().strftime("%B %d, %Y").replace(" 0", " ")),
                 ("\\quad", " "), ("\\hfill", " "), ("\\ ", " "), ("\\\\", "\x00")]:
        t = t.replace(a, b)
    t = t.replace("---", "—").replace("--", "–").replace("``", "“").replace("''", "”")
    t = re.sub(r"(?<![\w])`", "‘", t).replace("'", "’")
    t = t.replace("~", "\u00a0")
    t = re.sub(r"\s+", " ", t).replace("\x00", "\n")
    return t

# ---------------------------------------------------------------- inline

def inline(s, fmt=None):
    """Return a list of runs (text, fmt) for a LaTeX fragment."""
    fmt = dict(fmt or {})
    runs = []
    i = 0
    buf = ""

    def flush():
        nonlocal buf
        if buf:
            runs.append((plain(buf), dict(fmt)))
            buf = ""

    while i < len(s):
        c = s[i]
        if c == "$":
            j = s.index("$", i + 1)
            buf += math_to_text(s[i + 1:j])
            i = j + 1
            continue
        if c in "{}":
            i += 1
            continue
        if c == "\\":
            m = re.match(r"\\([A-Za-z]+)", s[i:])
            if not m:
                buf += s[i:i + 2]
                i += 2
                continue
            name = m.group(1)
            k = i + len(m.group(0))
            styles = {"textbf": {"b": True}, "emph": {"i": True}, "textit": {"i": True},
                      "texttt": {"code": True}, "code": {"code": True}, "commit": {"code": True},
                      "small": {}, "todo": {"b": True, "color": RED}, "mine": {"b": True, "color": BLUE}}
            if name in ("textcolor",):
                j = matching_brace(s, k)
                color = s[k + 1:j]
                k2 = matching_brace(s, j + 1)
                flush()
                runs += inline(s[j + 2:k2], {**fmt, "color": RED if color == "todo" else BLUE if color == "mine" else None})
                i = k2 + 1
                continue
            if name == "url":
                j = matching_brace(s, k)
                flush()
                runs.append((s[k + 1:j], dict(fmt)))
                i = j + 1
                continue
            if name == "href":
                j = matching_brace(s, k)
                url = s[k + 1:j]
                k2 = matching_brace(s, j + 1)
                text = s[j + 2:k2]
                flush()
                runs += inline(text, fmt)
                if not url.startswith("mailto:") and plain(text) != url:
                    runs.append((f" ({url})", dict(fmt)))
                i = k2 + 1
                continue
            if name in styles and k < len(s) and s[k] == "{":
                j = matching_brace(s, k)
                flush()
                inner = s[k + 1:j]
                if name == "todo":
                    inner = "[" + inner + "]"
                if name == "mine":
                    inner = "[Arash: " + inner + "]"
                runs += inline(inner, {**fmt, **styles[name]})
                i = j + 1
                continue
            if name in ("small", "noindent", "medskip", "par", "sloppy", "centering"):
                i = k
                continue
            # character macros handled by plain()
            buf += m.group(0)
            i = k
            continue
        buf += c
        i += 1
    flush()
    return runs

# ---------------------------------------------------------------- blocks

def split_paragraphs(text):
    return [p.strip() for p in re.split(r"\n\s*\n", text) if p.strip()]


def table_body(inner):
    """The rows of a tabular, tabularx or longtable, without its arguments."""
    m = re.search(r"\\begin\{(tabularx|tabular)\}", inner)
    if m:
        end = inner.index("\\end{" + m.group(1) + "}")
        rest = inner[m.end():end]
        nargs = 2 if m.group(1) == "tabularx" else 1
    else:
        rest = inner.strip()
        nargs = 1
    for _ in range(nargs):
        rest = rest.lstrip()
        j = matching_brace(rest, 0)
        rest = rest[j + 1:]
    return rest


def parse_table(body):
    body = re.sub(r"\\(toprule|midrule|bottomrule|endhead)", "", body)
    rows = []
    for row in re.split(r"\\\\\s*\n?", body):
        row = row.strip()
        if not row:
            continue
        cells = re.split(r"(?<!\\)&", row)
        rows.append([c.strip() for c in cells])
    return rows


def strip_stray_braces(p):
    """Remove the unmatched braces of a group such as {\\small ...} split across blocks."""
    p = p.strip()
    while p.startswith("{") and p.count("{") > p.count("}"):
        p = p[1:].strip()
    while p.endswith("}") and p.count("}") > p.count("{"):
        p = p[:-1].strip()
    return p


def parse_blocks(text, ctx):
    """Turn a body of LaTeX into a list of blocks."""
    blocks = []
    pos = 0
    env_re = re.compile(r"\\begin\{(reviewer|response|itemize|quote|center|longtable|flushleft|tcolorbox)\}"
                        r"|\\group\{|\\status\{|\\newpage|\\tableofcontents")
    while True:
        m = env_re.search(text, pos)
        chunk = text[pos:m.start() if m else len(text)]
        for p in split_paragraphs(chunk):
            p = strip_stray_braces(p)
            if p:
                blocks.append(("p", inline(p)))
        if not m:
            break
        tok = m.group(0)
        if tok == "\\newpage" or tok == "\\tableofcontents":
            if tok == "\\newpage":
                blocks.append(("pagebreak",))
            pos = m.end()
            continue
        if tok == "\\group{":
            j = matching_brace(text, m.end() - 1)
            ctx["group"] += 1
            ctx["cmt"] = 0
            blocks.append(("h1", f"{ctx['group']}.  {plain(text[m.end():j])}"))
            pos = j + 1
            continue
        if tok == "\\status{":
            j = matching_brace(text, m.end() - 1)
            blocks.append(("p", [("Status: ", {"i": True})] + inline(text[m.end():j])))
            pos = j + 1
            continue
        env = m.group(1)
        end = text.index("\\end{" + env + "}", m.end())
        inner = text[m.end():end]
        pos = end + len("\\end{" + env + "}")
        if env == "reviewer":
            j = matching_brace(inner, 0)
            source = plain(inner[1:j])
            ctx["cmt"] += 1
            blocks.append(("comment", f"Comment {ctx['group']}.{ctx['cmt']}", source,
                           [inline(p) for p in split_paragraphs(inner[j + 1:])]))
        elif env == "response":
            sub = parse_blocks(inner, ctx)
            if sub and sub[0][0] == "p":
                sub[0] = ("p", [("Response. ", {"b": True})] + sub[0][1])
            else:
                sub.insert(0, ("p", [("Response.", {"b": True})]))
            blocks += sub
        elif env == "itemize":
            inner = re.sub(r"^\[[^\]]*\]", "", inner.strip())
            for item in inner.split("\\item")[1:]:
                blocks.append(("bullet", inline(item.strip())))
        elif env == "quote":
            for line in inner.split("\\\\"):
                line = re.sub(r"\\(small|ttfamily)", "", line).strip()
                if line:
                    blocks.append(("mono", inline(line)))
        elif env in ("center", "longtable"):
            body = table_body(inner)
            rows = parse_table(body)
            blocks.append(("table", [[inline(c) for c in r] for r in rows]))
        elif env == "flushleft":
            for part in re.split(r"\n\s*\n|\\medskip", inner):
                part = part.strip()
                if not part:
                    continue
                for line in re.split(r"\\\\", part):
                    if line.strip():
                        blocks.append(("line", inline(line.strip())))
                blocks.append(("line", []))
        elif env == "tcolorbox":
            inner = re.sub(r"^\[[^\]]*\]", "", inner.strip())
            blocks.append(("note", [inline(p) for p in split_paragraphs(inner)]))
    return blocks

# ---------------------------------------------------------------- XML

def run_xml(text, fmt):
    props = []
    if fmt.get("b"):
        props.append("<w:b/>")
    if fmt.get("i"):
        props.append("<w:i/>")
    if fmt.get("code"):
        props.append('<w:rFonts w:ascii="Consolas" w:hAnsi="Consolas" w:cs="Consolas"/><w:sz w:val="19"/>')
    if fmt.get("color"):
        props.append(f'<w:color w:val="{fmt["color"]}"/>')
    rpr = f"<w:rPr>{''.join(props)}</w:rPr>" if props else ""
    out = []
    for k, part in enumerate(text.split("\n")):
        if k:
            out.append(f"<w:r>{rpr}<w:br/></w:r>")
        if part:
            out.append(f'<w:r>{rpr}<w:t xml:space="preserve">{escape(part)}</w:t></w:r>')
    return "".join(out)


def para_xml(runs, style=None, ppr_extra="", keep_next=False):
    ppr = ""
    if style:
        ppr += f'<w:pStyle w:val="{style}"/>'
    if keep_next:
        ppr += "<w:keepNext/>"
    ppr += ppr_extra
    return f"<w:p><w:pPr>{ppr}</w:pPr>{''.join(run_xml(t, f) for t, f in runs)}</w:p>"


BOX = ('<w:pBdr><w:top w:val="single" w:sz="4" w:space="4" w:color="7FB19D"/>'
       '<w:left w:val="single" w:sz="4" w:space="4" w:color="7FB19D"/>'
       '<w:bottom w:val="single" w:sz="4" w:space="4" w:color="7FB19D"/>'
       '<w:right w:val="single" w:sz="4" w:space="4" w:color="7FB19D"/></w:pBdr>')


def table_xml(rows, width=9360):
    ncol = max(len(r) for r in rows)
    lens = [max(sum(len(t) for t, _ in r[c]) if c < len(r) else 0 for r in rows) for c in range(ncol)]
    weights = [max(6, min(l, 70)) for l in lens]
    cols = [int(width * w / sum(weights)) for w in weights]
    cols[-1] = width - sum(cols[:-1])
    grid = "".join(f'<w:gridCol w:w="{w}"/>' for w in cols)
    out = [f'<w:tbl><w:tblPr><w:tblW w:w="{width}" w:type="dxa"/><w:tblLayout w:type="fixed"/>'
           '<w:tblBorders><w:top w:val="single" w:sz="8" w:color="444444"/>'
           '<w:bottom w:val="single" w:sz="8" w:color="444444"/>'
           '<w:insideH w:val="single" w:sz="2" w:color="CCCCCC"/></w:tblBorders>'
           '<w:tblCellMar><w:left w:w="80" w:type="dxa"/><w:right w:w="80" w:type="dxa"/></w:tblCellMar>'
           f'</w:tblPr><w:tblGrid>{grid}</w:tblGrid>']
    for r_i, row in enumerate(rows):
        trpr = "<w:trPr><w:tblHeader/></w:trPr>" if r_i == 0 else "<w:trPr><w:cantSplit/></w:trPr>"
        out.append(f"<w:tr>{trpr}")
        for c in range(ncol):
            runs = row[c] if c < len(row) else []
            if r_i == 0:
                runs = [(t, {**f, "b": True}) for t, f in runs]
            shade = '<w:shd w:val="clear" w:color="auto" w:fill="E6EFEA"/>' if r_i == 0 else ""
            out.append(f'<w:tc><w:tcPr><w:tcW w:w="{cols[c]}" w:type="dxa"/>{shade}</w:tcPr>'
                       f'{para_xml(runs, "TableText")}</w:tc>')
        out.append("</w:tr>")
    out.append("</w:tbl>")
    out.append(para_xml([], "TableText"))
    return "".join(out)


def blocks_xml(blocks):
    out = []
    for b in blocks:
        kind = b[0]
        if kind == "p":
            out.append(para_xml(b[1]))
        elif kind == "line":
            out.append(para_xml(b[1], "Line"))
        elif kind == "h1":
            out.append(para_xml([(b[1], {})], "Heading1"))
        elif kind == "pagebreak":
            out.append('<w:p><w:r><w:br w:type="page"/></w:r></w:p>')
        elif kind == "bullet":
            out.append(para_xml(b[1], "ListBullet", '<w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>'))
        elif kind == "mono":
            out.append(para_xml([(t, {**f, "code": True}) for t, f in b[1]], "Quote"))
        elif kind == "table":
            out.append(table_xml(b[1]))
        elif kind == "note":
            shade = '<w:shd w:val="clear" w:color="auto" w:fill="FBEDED"/>'
            bdr = BOX.replace("7FB19D", "C86464")
            for runs in b[1]:
                out.append(para_xml(runs, "BoxText", bdr + shade))
        elif kind == "comment":
            _, title, source, paras = b
            shade = '<w:shd w:val="clear" w:color="auto" w:fill="DCEBE4"/>'
            out.append(para_xml([(title, {"b": True}), ("    " + source, {"i": True})], "BoxTitle",
                                BOX + shade, keep_next=True))
            shade = '<w:shd w:val="clear" w:color="auto" w:fill="F2F5F8"/>'
            for runs in paras:
                out.append(para_xml(runs, "BoxText", BOX + shade, keep_next=True))
    return "".join(out)


STYLES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Calibri" w:hAnsi="Calibri" w:cs="Calibri" w:eastAsia="Calibri"/><w:sz w:val="22"/><w:lang w:val="en-US"/></w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr><w:spacing w:after="120" w:line="264" w:lineRule="auto"/></w:pPr></w:pPrDefault></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/><w:qFormat/></w:style>
<w:style w:type="paragraph" w:styleId="Heading1"><w:name w:val="heading 1"/><w:basedOn w:val="Normal"/><w:next w:val="Normal"/><w:qFormat/>
<w:pPr><w:keepNext/><w:spacing w:before="360" w:after="160"/><w:outlineLvl w:val="0"/></w:pPr><w:rPr><w:b/><w:color w:val="00643C"/><w:sz w:val="30"/></w:rPr></w:style>
<w:style w:type="paragraph" w:styleId="Line"><w:name w:val="Address line"/><w:basedOn w:val="Normal"/><w:pPr><w:spacing w:after="0"/></w:pPr></w:style>
<w:style w:type="paragraph" w:styleId="BoxTitle"><w:name w:val="Comment title"/><w:basedOn w:val="Normal"/><w:pPr><w:spacing w:before="200" w:after="0"/><w:ind w:left="120" w:right="120"/></w:pPr><w:rPr><w:sz w:val="20"/></w:rPr></w:style>
<w:style w:type="paragraph" w:styleId="BoxText"><w:name w:val="Comment text"/><w:basedOn w:val="Normal"/><w:pPr><w:spacing w:after="0"/><w:ind w:left="120" w:right="120"/></w:pPr></w:style>
<w:style w:type="paragraph" w:styleId="ListBullet"><w:name w:val="List Bullet"/><w:basedOn w:val="Normal"/><w:pPr><w:spacing w:after="40"/></w:pPr></w:style>
<w:style w:type="paragraph" w:styleId="Quote"><w:name w:val="Quote"/><w:basedOn w:val="Normal"/><w:pPr><w:spacing w:after="0"/><w:ind w:left="720"/></w:pPr></w:style>
<w:style w:type="paragraph" w:styleId="TableText"><w:name w:val="Table text"/><w:basedOn w:val="Normal"/><w:pPr><w:spacing w:before="40" w:after="40" w:line="240" w:lineRule="auto"/></w:pPr><w:rPr><w:sz w:val="19"/></w:rPr></w:style>
<w:style w:type="paragraph" w:styleId="Footer"><w:name w:val="footer"/><w:basedOn w:val="Normal"/><w:pPr><w:jc w:val="right"/></w:pPr><w:rPr><w:sz w:val="18"/><w:color w:val="666666"/></w:rPr></w:style>
</w:styles>"""

NUMBERING = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:numbering xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:abstractNum w:abstractNumId="0"><w:multiLevelType w:val="singleLevel"/>
<w:lvl w:ilvl="0"><w:start w:val="1"/><w:numFmt w:val="bullet"/><w:lvlText w:val="\u2022"/><w:lvlJc w:val="left"/>
<w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr><w:rPr><w:rFonts w:ascii="Symbol" w:hAnsi="Symbol" w:hint="default"/></w:rPr></w:lvl></w:abstractNum>
<w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num>
</w:numbering>"""

FOOTER = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:ftr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:p><w:pPr><w:pStyle w:val="Footer"/></w:pPr>
<w:r><w:t xml:space="preserve">@TITLE@ — page </w:t></w:r>
<w:r><w:fldChar w:fldCharType="begin"/></w:r><w:r><w:instrText xml:space="preserve"> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType="separate"/></w:r><w:r><w:t>1</w:t></w:r><w:r><w:fldChar w:fldCharType="end"/></w:r>
</w:p></w:ftr>"""

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/numbering.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"/>
<Override PartName="/word/footer1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/>
<Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>
</Types>"""

RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/>
</Relationships>"""

DOC_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering" Target="numbering.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer" Target="footer1.xml"/>
</Relationships>"""


def core_xml(title):
    now = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" '
            'xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" '
            'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">'
            f'<dc:title>{escape(title)}</dc:title><dc:creator>Arash Massoudieh</dc:creator>'
            f'<dcterms:created xsi:type="dcterms:W3CDTF">{now}</dcterms:created></cp:coreProperties>')


def main(tex_path, docx_path):
    tex = open(tex_path, encoding="utf-8").read()
    # The footer and document title come from the LaTeX \lhead
    m = re.search(r"\\lhead\{\\small\s*([^}]*)\}", tex)
    title = m.group(1).strip() if m else "SedSAT3"
    body = tex[tex.index("\\begin{document}") + len("\\begin{document}"):tex.index("\\end{document}")]
    body = re.sub(r"(?m)^%.*$", "", body)
    body = re.sub(r"\\sloppy|\\setlength\{[^}]*\}\{[^}]*\}", "", body)
    blocks = parse_blocks(body, {"group": 0, "cmt": 0})
    document = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
                'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"><w:body>'
                + blocks_xml(blocks) +
                '<w:sectPr><w:footerReference w:type="default" r:id="rId3"/>'
                '<w:pgSz w:w="12240" w:h="15840"/><w:pgMar w:top="1440" w:right="1440" w:bottom="1440" '
                'w:left="1440" w:header="720" w:footer="720" w:gutter="0"/></w:sectPr></w:body></w:document>')
    with zipfile.ZipFile(docx_path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/document.xml", document)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/numbering.xml", NUMBERING)
        z.writestr("word/footer1.xml", FOOTER.replace("@TITLE@", escape(title)))
        z.writestr("docProps/core.xml", core_xml(title))
    print(f"{docx_path}: {len(blocks)} blocks")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
