# ndspatcher-reversings
Series of various legacy programs that, either from the side of the PC
or the side of the NDS, patched card reads such that a .nds image could
be exeucted from a slot2 card/controller, rather than the expected slot1.

Repository structure:
- /maxoverload/exe_v1-2/ <br/>
Results of reversing the Maxoverload patcher, used for a Mass Media Dock
(requiring a HDD/CF card, using an IDE protocol). Generates a .PAT file,
and uses its own .nds to init another .nds corresponding to the .PAT.
Compatibility: Up to ID# 556 at least, but nothing above ID# 1000.
(with testing I've done, this matches the original's functionality)
(note: the reported build date for this is May 16, 2006)
- /maxoverload/nds_v1-61b2/ <br/>
[Work in Progress]
Results of reversing the most up-to-date (it seems to be from around
2010) version of Maxoverload, v1.61b2, which does not have any PC 
side tool and is simply ran through a Mass Media Dock, and seemingly 
on-the-fly patches other .nds images. (This will likely take longer 
as it's entirely ARM, unlike x86 I'm most familiar with)
So far it _seems_ like this used libnds with its own filesystem driver.
- /ndspatch/ <br/>
Results of reversing the patcher by ]{ain, this directly modifies the
previously existing .nds, converting it to a .nds.gba; which basically
patches certain functions and adds a GBA header to the very top.
Compatibility: Up to ID# 200, very low.
(this is incomplete for now, only matches functionality with -nosave
 and games without compression; deemed the rest unnecessary)

Oct/7/2016: Cleaned up the repository, sorry for any confusion.
