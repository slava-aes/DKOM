## Windows 10 - Direct Kernel Object Manipulation


PatchGuard takes some minutes to cause a bugcheck

### The ActiveProcessLink offset is hardcoded for this windows build.

#### This is not required, however. 
#### Although this offset might change in the next version, it's  **always** after *UniqueProcessId* 

One can check this by using WinDbg to dump [_EPROCESS](https://www.vergiliusproject.com/kernels/x64/Windows%2010%20|%202016/1809%20Redstone%205%20(October%20Update)/_EPROCESS).


For historic view of how these offsets have been changing across diferent windows versions see [here](https://www.vergiliusproject.com)