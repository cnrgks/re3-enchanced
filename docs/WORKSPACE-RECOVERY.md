# Restoring the development workspace

The public fork contains the source and submodule references. The workspace tools, runtime dependencies, reference mods and full historical release packages are in the [private workspace backup](https://github.com/cnrgks/hezkore-re3-pdb-build/releases/tag/workspace-backup-2026-11-20). Sign into the GitHub account with access to the private build repository.

1. Create an empty workspace folder and extract all three backup ZIPs into it. This restores the root instructions, `tools/`, `dependencies/`, `references/`, `releases/` and `archive/legacy/`.
2. From that folder, clone the public fork with its submodules.

   ```powershell
   git clone --recurse-submodules https://github.com/cnrgks/re3-enchanced.git hez-gta-re3
   ```

3. Read the restored root `README.md` and `AGENTS.md` for the build and packaging workflow. The private build repository runs GitHub Actions from its `enhanced-source` branch. Sync that branch to the intended source commit before dispatching a build.
4. To restore an expanded historical release, extract that version's original ZIP into `releases/`. Each original ZIP contains its own version folder.

GitHub credentials, the GTA III installation, saves and personal game settings belong to the local machine. Build staging, downloaded build artifacts and temporary logs can be regenerated.
