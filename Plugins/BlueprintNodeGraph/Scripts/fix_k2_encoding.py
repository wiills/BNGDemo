# -*- coding: utf-8 -*-
"""Fix K2Nodes source encoding: ASCII-only comments, valid UTF-8."""
import re
from pathlib import Path

K2 = Path(__file__).resolve().parents[1] / "Source/BlueprintNodeGraphEditor"
DIRS = [
    K2 / "Public/BlueprintTool/K2Nodes",
    K2 / "Private/BlueprintTool/K2Nodes",
]

# Per-file header / block comment replacements (full line or block)
HEADER_COMMENTS = {
    "ExK2Node_WaitAll.h": '/** Multi-branch merge: continue after all branches report (EExBranchMode::All). */',
    "ExK2Node_WaitAny.h": '/** Multi-branch merge: continue after any branch succeeds (EExBranchMode::Any). */',
    "ExK2Node_WaitCount.h": '/** Multi-branch merge: continue after success count reaches Required Success Count (EExBranchMode::Count). */',
    "ExK2Node_QuestTask.h": (
        "/**\n * @class UExK2Node_QuestTask\n"
        " * @brief Quest latent task K2 node (CreateQuestProxy on UExLatentTask_Quest, not CreateLatentTask).\n */"
    ),
    "ExK2Node_LoadAsset.h": (
        "/**\n * @class UExK2Node_LoadAsset\n"
        " * @brief Async load asset blueprint node (path or class-based load).\n */"
    ),
    "ExK2Node_StreamLevel.h": (
        "/**\n * @class UExK2Node_StreamLevel\n"
        " * @brief Async stream load/unload level with optional delay and completion callback.\n */"
    ),
    "ExK2Node_ShowBase.h": "/** Base K2 node for editor display; provides GetMenuActions. */",
}

# Line comment replacements (substring match -> replacement line suffix)
LINE_FIXES = [
    (r"//\s*.*[\u0080-\uffff].*", None),  # strip handled below
    ("// Reload node style", "// Override node visual style"),
    ("// most important", "// Register node creation in context menu"),
    ("// é‡è½½Nodeæ ·å¼", "// Override node visual style"),
    ("// most importantï¼šæŠŠè¿™ä¸ªèŠ‚ç‚¹çš„åˆ›å»ºæ“ä½œæ·»åŠ åˆ°å³é”®èœå•ä¸?", "// Register this node in the context menu"),
]

ASYNC_BASE_H_BLOCK = '''/**
 * @class UExK2Node_AsyncBase
 * @brief Base editor class for custom async blueprint nodes (extends UK2Node_BaseAsyncTask).
 *
 * Node info, UUID, compiler expansion, delegate wiring.
 * @see UK2Node_BaseAsyncTask, FExLatentNodeInfo
 */'''

def strip_non_ascii_in_comments(text: str) -> str:
    """Remove/replace non-ASCII inside // and /* */ comments only."""
    out = []
    i = 0
    n = len(text)
    while i < n:
        if text.startswith("/*", i):
            end = text.find("*/", i + 2)
            if end == -1:
                out.append(text[i:])
                break
            block = text[i : end + 2]
            if any(ord(c) > 127 for c in block):
                # keep only if already clean
                clean = "".join(c if ord(c) < 128 else " " for c in block)
                clean = re.sub(r" +", " ", clean)
                clean = re.sub(r"\n\s+\n", "\n", clean)
                block = clean
            out.append(block)
            i = end + 2
        elif text.startswith("//", i):
            end = text.find("\n", i)
            if end == -1:
                line = text[i:]
                i = n
            else:
                line = text[i:end]
                i = end
            if any(ord(c) > 127 for c in line):
                # drop non-ascii from line comment
                line = "".join(c if ord(c) < 128 else "" for c in line).rstrip()
                if line.strip() == "//":
                    line = ""
            out.append(line)
        else:
            # copy until next comment or string
            j = i
            while j < n:
                if text.startswith("/*", j) or text.startswith("//", j):
                    break
                if text[j] == '"':
                    j += 1
                    while j < n and text[j] != '"':
                        if text[j] == "\\":
                            j += 2
                        else:
                            j += 1
                    if j < n:
                        j += 1
                    continue
                j += 1
            out.append(text[i:j])
            i = j
    return "".join(out)


def fix_file(path: Path) -> bool:
    raw = path.read_bytes()
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError:
        # fix common truncation: fullwidth paren + corrupted CJK tail
        text = raw.decode("latin-1")
        text = text.replace("\xe3\x80?", ".")
        text = text.replace("\xef\xbc\x89\xe3\x80?", ").")
        text = text.replace("\xe3\x80\x82", ".")

    orig = text
    name = path.name

    if name in HEADER_COMMENTS:
        rep = HEADER_COMMENTS[name]
        if name.endswith(".h") and "/**" in text:
            text = re.sub(r"/\*\*.*?\*/", rep, text, count=1, flags=re.DOTALL)

    if name == "ExK2Node_AsyncBase.h":
        text = re.sub(r"/\*\*.*?@see UK2Node_BaseAsyncTask.*?\*/", ASYNC_BASE_H_BLOCK, text, count=1, flags=re.DOTALL)
        # fix member doc lines with mojibake
        replacements = {
            "/** SetK2NodeInfo function name */": "/** SetK2NodeInfo function name */",
            "/** Node configuration */": "/** Node configuration (NodeInfo) */",
        }
        for old_pat, new in [
            (r"/\*\* SetK2NodeInfo.*?\*/", "/** SetK2NodeInfo function name */"),
            (r"/\*\* .*NodeInfo.*?\*/\s*\n\s*UPROPERTY\(EditAnywhere", "/** Node configuration */\n\tUPROPERTY(EditAnywhere"),
            (r"/\*\*\s*\n\s*\* @brief .*?ObjectInitializer.*?\n\s*\*/", "/** Constructor */"),
            (r"/\*\* Factory unset.*?ensure.*?\*/", "/** Safe tooltip when proxy factory is not set yet */"),
            (r"/\*\* PIE/SIE.*?bubble.*?\*/", "/** Debug timing bubble in PIE/SIE (via debug subsystem) */"),
            (r"/\*\*\s*\n\s*\* @brief Whether to show.*?\*/", "/** Show node properties in details panel */"),
            (r"/\*\*\s*\n\s*\* @brief After property edit.*?\*/", "/** PostEditChangeProperty */"),
            (r"/\*\*\s*\n\s*\* @brief Get input branch count.*?\*/", "/** Number of exec input branches */"),
            (r"/\*\*\s*\n\s*\* @brief Get node UUID.*?\*/", "/** UUID string from NodeGuid hash */"),
            (r"/\*\*\s*\n\s*\* @brief Set node info.*?\*/", "/** Write NodeInfo to pin default */"),
            (r"/\*\*\s*\n\s*\* @brief Set UUID.*?\*/", "/** Set UUID and NodeInfo pins */"),
            (r"/\*\*\s*\n\s*\* @brief Generate context.*?\*/", "/** Build ContextUniqueId from node UUID and owner */"),
            (r"/\*\*\s*\n\s*\* @brief Expand node.*?\*/", "/** Kismet expand entry */"),
            (r"/\*\*\s*\n\s*\* @brief Compiler.*SetK2NodeInfo.*?\*/", "/** Emit SetK2NodeInfo call */"),
            (r"/\*\*\s*\n\s*\* @brief Compiler.*Activate.*?\*/", "/** Emit Activate call */"),
            (r"/\*\*\s*\n\s*\* @brief Compiler.*delegate.*?\*/", "/** Wire delegate bindings and assignments */"),
        ]:
            text = re.sub(old_pat, new, text, count=1, flags=re.DOTALL)

    for old, new in [
        ("// é‡è½½Nodeæ ·å¼", "// Override node visual style"),
        ("// most importantï¼šæŠŠè¿™ä¸ªèŠ‚ç‚¹çš„åˆ›å»ºæ“ä½œæ·»åŠ åˆ°å³é”®èœå•ä¸?", "// Register this node in the context menu"),
        ("// most important", "// Register this node in the context menu"),
    ]:
        if old in text:
            text = text.replace(old, new)

    text = strip_non_ascii_in_comments(text)

    # ensure valid utf-8
    text.encode("utf-8")

    if text != orig:
        path.write_text(text, encoding="utf-8", newline="\n")
        return True
    return False


changed = []
for d in DIRS:
    for p in sorted(d.glob("*")):
        if p.suffix in (".h", ".cpp"):
            if fix_file(p):
                changed.append(p.name)

print("fixed:", len(changed), changed)
