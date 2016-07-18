# pdftohtml

This is a fork of pdftohtml 0.40a.  It was originally created so that I could
[fix long-standing XML output errors](http://marc.info/?l=pdftohtml-general&m=123159307310803&q=p3),
but in the process I discovered that the released code for 0.40a wasn't even in
the CVS tree at SourceForge, so I merged that into the version history here.

Unfortunately, the `pdftohtml` included directly in the `xpdf` distribution
does not have features such as `-noframes`, `-xml`, etc.
