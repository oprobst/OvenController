"""Creates an empty KiCad 10 board file: 2 copper layers, 1.6 mm FR-4, 35 um copper (T019).

Run with KiCad's bundled Python:
    "D:/hobby/Elektronik/KiCad/10.0/bin/python.exe" hardware/tools/make_board.py hardware/controller/controller.kicad_pcb

Refuses to overwrite an existing board: the layout is edited in KiCad from then on.
Saving also rewrites <board>.kicad_pro with defaults, so run make_project.py afterwards.
"""

import sys
from pathlib import Path

import pcbnew


def main(target):
    target = Path(target)
    if target.exists():
        sys.exit(f"{target} exists, not overwritten")
    board = pcbnew.BOARD()
    board.SetCopperLayerCount(2)
    ds = board.GetDesignSettings()
    ds.SetBoardThickness(pcbnew.FromMM(1.6))
    pcbnew.SaveBoard(str(target), board)
    print(f"written {target}")


if __name__ == "__main__":
    main(sys.argv[1])
