# Leviathan
Cross-platform GM Tool for SphereServer.<br>
Non-windows versions lack, for now, the "send text to client" feature.<br>
Consider it at an embryonal stage, since it lacks lots of features, and a slow-progressing project.<br>
Some things could have be done more easily using Qt classes, but i'd rather write code that can be recycled for other projects (which aren't necessarily using Qt).<br>

# Compilation
* Windows: The preferred way is to use Qt Creator to open the .pro file and compile it.
* Linux: Use Qt Creator, or you can compile it under command line using qmake and make.
* Mac: Same as Linux, but you may need to manually install zlib (libz.dylib) via `brew install zlib`.
   If you don't have that, you'll get linker errors.

# Credits
Uses Qt Toolkit 5.<br>
Uses CheckableProxyModel (GPLv3 or later versions, as of 2011) by Andre Somers.<br>
Used as code base for UOP files handling the UO_Package library by Kons (https://code.google.com/archive/p/kprojects/downloads).<br>
<br>
References for UO files formats:
* http://wpdev.sourceforge.net/docs/formats/csharp/default.html
* http://uo.stratics.com/heptazane/fileformats.shtml

Got help (or whole pieces of code) from:
* AxisII by Ben Croussette (https://github.com/Ben1028/Axis2)
* Punt's C++/Qt4 Ultima SDK: http://www.ryandor.com/files/punt/3%20-%20Other%20Files%20&%20Utilities/Punt's%20Ultima%20C++%20SDK/
* UOFiddler/C# (original) Ultima SDK: https://github.com/msturgill/uofiddler

# License
Licensed under GPL v3.
