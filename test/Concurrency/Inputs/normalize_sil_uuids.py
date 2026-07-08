"""Normalize per-compilation archetype UUIDs in SIL for stable diffs."""

import re
import sys

HEX = r'[0-9A-F]'
UUID = rf'{HEX}{{8}}-{HEX}{{4}}-{HEX}{{4}}-{HEX}{{4}}-{HEX}{{12}}'

text = sys.stdin.read()
text = re.sub(rf'@opened\("{UUID}"', '@opened("UUID"', text)
text = re.sub(rf'@pack_element\("{UUID}"', '@pack_element("UUID"', text)
text = re.sub(rf'uuid "{UUID}"', 'uuid "UUID"', text)
sys.stdout.write(text)
