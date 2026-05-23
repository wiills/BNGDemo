
# -*- coding: utf-8 -*-
"""Fix Docs/*.md encoding by reading and re-saving as UTF-8."""
from pathlib import Path

DOCS = Path(__file__).resolve().parents[1] / "Docs"


def fix_file(file_path):
    """Read a file and re-save as UTF-8."""
    print(f"\nProcessing {file_path.name}...")
    
    try:
        raw = file_path.read_bytes()
        
        # Try common encodings in order
        text = None
        tried = []
        
        for encoding in ['utf-8-sig', 'utf-8', 'gbk', 'gb18030', 'cp1252']:
            tried.append(encoding)
            try:
                text = raw.decode(encoding)
                print(f"  ✓ Decoded with {encoding}")
                break
            except UnicodeDecodeError:
                continue
        
        if text is None:
            # Last resort: replace errors
            text = raw.decode('utf-8', errors='replace')
            print(f"  ⚠️ Decoded with UTF-8 (replace errors)")
        
        # Re-save as UTF-8 with LF newlines (no BOM)
        file_path.write_text(text, encoding='utf-8', newline='\n')
        print(f"  ✓ Saved as UTF-8 (LF, no BOM)")
        return True
        
    except Exception as e:
        print(f"  ✗ Error: {e}")
        return False


if __name__ == "__main__":
    print("=" * 50)
    print("Fixing Docs directory encoding")
    print("=" * 50)
    
    success_count = 0
    fail_count = 0
    
    for file_path in sorted(DOCS.glob("*.md")):
        if fix_file(file_path):
            success_count += 1
        else:
            fail_count += 1
    
    print("\n" + "=" * 50)
    print(f"Done: {success_count} succeeded, {fail_count} failed")
    print("=" * 50)
