## Windows 10 - Direct Kernel Object Manipulation


PatchGuard takes some minutes to cause a bugcheck

### The ActiveProcessLink offset is hardcoded for this windows build.

#### This is not required, however. 
#### Although this offset might change in the next version, it's  **always** after *UniqueProcessId* 

One can check this by using WinDbg to dump _EPROCESS.


For historic view of how these offsets have been changing across diferent windows versions see [here](https://www.vergiliusproject.com)