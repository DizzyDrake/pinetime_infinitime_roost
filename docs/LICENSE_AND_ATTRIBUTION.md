# License and provenance

**Original Roost additions:** Aurora selected Apache License 2.0 for applicable original projects. The repository includes the unmodified Apache-2.0 text in `LICENSE` for independently licensable original contributions. No individual copyright identity has been invented.

**InfiniTime integration:** This repository is designed for InfiniTime 1.16.0 at commit `0fabfe99d9115c5e258a14fd0c985d300ee41f2d`. Upstream InfiniTime is GPLv3. The patches modify upstream GPL-governed files, and a combined redistributed firmware must comply with upstream GPLv3 obligations; an Apache-2.0 notice does **not** relicense upstream source or the firmware binary. A reference copy of GPLv3 is included in `LICENSES/`, and the upstream project's authoritative license should be checked at publication.

**Artwork:** `artwork/infinitime_roost_icon.svg` is Aurora's unchanged supplied SVG (SHA-256 `21348ede866c135f4980b7ebfeec31c207eb11006fb3634658289ce85a348c3f`). Its embedded attribution reads Font Awesome Free 5.15.4 by @fontawesome, https://fontawesome.com; icon CC BY 4.0, https://creativecommons.org/licenses/by/4.0/. Aurora customized this SVG. The 18×18 private launcher glyph is derived from it; keep the attribution with redistributed icon artwork. No font file is distributed in this packet.

**Source provenance:** Four patch files are byte-identical to their existing ChatGPT Library originals. Nine Roost-specific files were reconstructed and verified from the original integration patches; the `docs/annotated-source/` tree only adds explicitly marked explanatory comments. The separate firmware release package is identified by its SHA-256 checksum; inspect the release notes for build provenance.

This document describes known source terms and the remaining rights review; it is not legal advice or a claim of completed public release clearance.
