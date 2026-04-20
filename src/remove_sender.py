import os
import re

def remove_sender():
    modified_count = 0
    patterns = [
        (r'QObject\s*\*\s*sender\s*,?\s*', ''),
        (r'Q_UNUSED\s*\(\s*sender\s*\);?\s*', ''),
        (r'sender,?\s*', ''),
        (r'if\s*\(\s*sender\s*!=\s*this\s*&&\s*', 'if ('),
    ]

    for root, dirs, files in os.walk('.'):
        for file in files:
            if file.endswith(('.cpp', '.h')):
                path = os.path.join(root, file)
                try:
                    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                    
                    new_content = content
                    for pattern, replacement in patterns:
                        new_content = re.sub(pattern, replacement, new_content)
                    
                    if new_content != content:
                        with open(path, 'w', encoding='utf-8', errors='ignore') as f:
                            f.write(new_content)
                        print(f"updated: {path}")
                        modified_count += 1
                except Exception as e:
                    print(f"Error processing {path}: {e}")

    print(f"\nTotal modified files: {modified_count}")
    print("\nremaining 'sender' references:")
    os.system('grep -rn "sender" .')

if __name__ == "__main__":
    remove_sender()
