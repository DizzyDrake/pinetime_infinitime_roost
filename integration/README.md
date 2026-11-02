# InfiniTime integration

The installer in `tools/install_into_infinitime.py` handles these changes automatically. They connect Roost to the InfiniTime application registry, long-lived service/controller, incoming-call and alarm paths, and launcher icon.

The two small integration diffs affect **existing InfiniTime files only**. The nine new Roost-specific files are copied from the repository's canonical `src/` directory. The original seven development patches are preserved separately under `history/` and are not required for setup.
