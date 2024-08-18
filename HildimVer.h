#define VERSION_HILDIM "8.2.2"

#ifdef IsHildiMApp
#define FILEDESCRIPTIONE FILEDESCRIPTION
#else
#define FILEDESCRIPTIONE FILEDESCRIPTION  "for Hildim " VERSION_HILDIM
#endif

#ifdef x64
#define VERSION_BIT " x64"
#else
#define VERSION_BIT " x32"
#endif
#ifdef Debug
#define _DEBUG
#define VERSIONEX VERSION_BIT " Debug"
#else
#define VERSIONEX VERSION_BIT
#endif


#define VERSION_IUP "3.31"
#define VERSION_HILDIM_W 8,2,2,0
#define FILEDESCRIPTIONEX FILEDESCRIPTIONE VERSIONEX
