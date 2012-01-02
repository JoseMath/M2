const char startupString[] =
"-- -*- coding: utf-8 -*-\n"
"--		Copyright 1993-2009 by Daniel R. Grayson\n"
"-- layout.m2, includes the former contents of ../m2/startup.m2\n"
"-- this file gets incorporated into the executable file bin/M2 as the string 'startupString'\n"
"-- we want to ignore the --datarootdir setting, whatever that means, so here we we mention it: ${prefix}/share\n"
"\n"
"interpreterDepth = loadDepth = errorDepth = 0\n"
"debuggingMode = true\n"
"stopIfError = false\n"
"debugWarningHashcode = null\n"
"gotarg := arg -> any(commandLine, s -> s == arg)\n"
"if gotarg \"--stop\" then stopIfError = true\n"
"\n"
"firstTime := class PackageDictionary === Symbol\n"
"\n"
"firstTime = true					    -- debugging\n"
"\n"
"if firstTime then (\n"
"     -- we do this bit *before* \"debug Core\", so that Core (the symbol, not the package), which may not be there yet, ends up in the right dictionary\n"
"     assert = x -> (\n"
"	  if class x =!= Boolean then error \"assert: expected true or false\";\n"
"	  if not x then error \"assertion failed\");\n"
"     PackageDictionary = new Dictionary;\n"
"     dictionaryPath = append(dictionaryPath,PackageDictionary);\n"
"     assert( not isGlobalSymbol \"Core\" );\n"
"     PackageDictionary#(\"Package$Core\") = getGlobalSymbol(PackageDictionary,\"Core\");\n"
"     ) \n"
"-- we can't make this an else-clause, because then \"Core\" will be in the wrong dictionary\n"
"disassemble ( () -> debug Core )\n"
"if not firstTime then debug Core -- we need access to the private symbols (we remove the Core private dictionary later.)\n"
"\n"
"toString := value getGlobalSymbol if firstTime then \"simpleToString\" else \"toString\"\n"
"match := X -> null =!= regex X\n"
"\n"
"local exe\n"
"local topSrcdir\n"
"local topBuilddir\n"
"\n"
"-- this next bit has to be *parsed* after the \"debug\" above, to prevent the symbols from being added to the User dictionary\n"
"if firstTime then (\n"
"     -- all global definitions go here, because after loaddata is run, we'll come through here again\n"
"     -- with all these already done and global variables set to read-only\n"
"\n"
"     filesLoaded = new MutableHashTable;\n"
"     loadedFiles = new MutableHashTable;\n"
"     notify = true;\n"
"     nobanner = false;\n"
"     texmacsmode = false;\n"
"     restarting = false;\n"
"     restarted = false;\n"
"     srcdirs = {};\n"
"\n"
"     markLoaded = (fullfilename,origfilename,notify,filetime) -> ( \n"
"	  fullfilename = minimizeFilename fullfilename;\n"
"	  filesLoaded#origfilename = (fullfilename,filetime); \n"
"	  loadedFiles##loadedFiles = realpath toAbsolutePath fullfilename; \n"
"	  if notify then stderr << \"--loaded \" << fullfilename << endl;\n"
"	  );\n"
"     normalPrompts = () -> (\n"
"	  lastprompt := \"\";\n"
"	  ZZ#{Standard,InputPrompt} = lineno -> concatenate(newline, lastprompt = concatenate(interpreterDepth:\"i\", toString lineno, \" : \"));\n"
"	  ZZ#{Standard,InputContinuationPrompt} = lineno -> #lastprompt; -- will print that many blanks, see interp.d\n"
"	  symbol currentPrompts <- normalPrompts;	    -- this avoids the warning about redefining a function\n"
"	  );\n"
"     normalPrompts();\n"
"     noPrompts = () -> (\n"
"	  ZZ#{Standard,InputPrompt} = lineno -> \"\";\n"
"	  ZZ#{Standard,InputContinuationPrompt} = lineno -> \"\";\n"
"	  symbol currentPrompts <- noPrompts;\n"
"	  );\n"
"\n"
"     startFunctions := {};\n"
"     addStartFunction = f -> ( startFunctions = append(startFunctions,f); f);\n"
"     runStartFunctions = () -> scan(startFunctions, f -> f());\n"
"\n"
"     endFunctions := {};\n"
"     addEndFunction = f -> ( endFunctions = append(endFunctions,f); f);\n"
"     runEndFunctions = () -> (\n"
"	  save := endFunctions;\n"
"	  endFunctions = {};\n"
"	  scan(save, f -> f());\n"
"	  endFunctions = save;\n"
"	  );\n"
"\n"
"     simpleExit := exit;\n"
"     exit = ret -> ( runEndFunctions(); simpleExit ret );\n"
"\n"
"     File << Thing  := File => (x,y) -> printString(x,toString y);\n"
"     File << Net := File << Symbol := File << String := printString;\n"
"     << Thing := x -> stdio << x;\n"
"     String | String := String => concatenate;\n"
"     Function _ Thing := Function => (f,x) -> y -> f splice (x,y);\n"
"     String | ZZ := String => (s,i) -> concatenate(s,toString i);\n"
"     ZZ | String := String => (i,s) -> concatenate(toString i,s);\n"
"\n"
"     new HashTable from List := HashTable => (O,v) -> hashTable v;\n"
"\n"
"     Manipulator = new Type of BasicList;\n"
"     Manipulator.synonym = \"manipulator\";\n"
"     new Manipulator from Function := Manipulator => (Manipulator,f) -> new Manipulator from {f};\n"
"     Manipulator Database := Manipulator File := Manipulator NetFile := (m,o) -> m#0 o;\n"
"\n"
"     Manipulator Nothing := (m,null) -> null;\n"
"     File << Manipulator := File => (o,m) -> m#0 o;\n"
"     NetFile << Manipulator := File => (o,m) -> m#0 o;\n"
"     Nothing << Manipulator := (null,m) -> null;\n"
"\n"
"     TeXmacsBegin = ascii 2;\n"
"     TeXmacsEnd   = ascii 5;\n"
"\n"
"     close = new Manipulator from close;\n"
"     closeIn = new Manipulator from closeIn;\n"
"     closeOut = new Manipulator from closeOut;\n"
"     flush = new Manipulator from flush;\n"
"     endl = new Manipulator from endl;\n"
"\n"
"     Thing#{Standard,Print} = x ->  (\n"
"	  << newline << concatenate(interpreterDepth:\"o\") << lineNumber << \" = \";\n"
"	  try << x;\n"
"	  << newline << flush;\n"
"	  );\n"
"\n"
"     first = x -> x#0;\n"
"     last = x -> x#-1;\n"
"     lines = x -> (\n"
"	  l := separate x;\n"
"	  if l#-1 === \"\" then drop(l,-1) else l);\n"
"\n"
"     isFixedExecPath = filename -> (\n"
"	  -- this is the way execvp(3) decides whether to search the path for an executable\n"
"	  match(\"/\", filename)\n"
"	  );\n"
"     re := \"/\";						    -- /foo/bar\n"
"     if version#\"operating system\" === \"MicrosoftWindows\" \n"
"     then re = re | \"|.:/\";				    -- \"C:/FOO/BAR\"\n"
"     re = re | \"|\\\\$\";					    -- $www.uiuc.edu:80\n"
"     re = re | \"|!\";					    -- !date\n"
"     re = re | \"|~/\";					    -- ~/foo/bar\n"
"     isAbsolutePathRegexp := \"^(\" | re | \")\";		    -- whether the path will work from any directory and get to the same file\n"
"     re = re | \"|\\\\./\";					    -- ./foo/bar\n"
"     re = re | \"|\\\\.\\\\./\";				    -- ../foo/bar\n"
"     isStablePathRegexp   := \"^(\" | re | \")\";               -- whether we should search only in the current directory (or current file directory)\n"
"     isAbsolutePath = filename -> match(isAbsolutePathRegexp, filename);\n"
"     isStablePath = filename -> match(isStablePathRegexp, filename);\n"
"     concatPath = (a,b) -> if isAbsolutePath b then b else a|b;\n"
"\n"
"     toAbsolutePath = pth -> if pth =!= \"stdio\" and not isAbsolutePath pth then \"/\" | relativizeFilename(\"/\", pth) else pth;\n"
"\n"
"     copyright = (\n"
"	  \"Macaulay2, version \" | version#\"VERSION\" | newline\n"
"	  | \"Copyright 2010 Daniel R. Grayson and Michael E. Stillman,\" | newline\n"
"     	  | \"  licensed under the GNU General Public License version 2 or 3.\" | newline\n"
"	  | \"For Macaulay2 packages, see their source code for licensing terms.\" | newline\n"
"     	  | \"Compiled with the following libraries:\" | newline\n"
"	  | \"  GC \" | version#\"gc version\" | \", by H. Boehm and Alan J. Demers\" | newline\n"
"	  | \"  Singular-Factory \" | version#\"factory version\" | \", by G.-M. Greuel et al.\" | newline\n"
"	  | \"  Singular-Libfac \" | version#\"libfac version\" | \", by M. Messollen\" | newline\n"
"	  |    ( \n"
"	       if version#\"frobby version\" =!= \"not present\" \n"
"	       then (\"  frobby \" | version#\"frobby version\" | \", by B. H. Roune\" | newline)\n"
"	       else \"\"\n"
"	       )\n"
"	  |    ( \n"
"	       if version#\"pari version\" =!= \"not present\" \n"
"	       then (\"  pari \" | version#\"pari version\" | \", the PARI Group, Bordeaux\" | newline)\n"
"	       else \"\"\n"
"	       )\n"
"	  | \"  NTL \" | version#\"ntl version\" | \", by V. Shoup\" | newline\n"
"	  |    (\n"
"	       if version#\"gmp version\" =!= \"not present\"\n"
"	       then \"  GNU MP \" | version#\"gmp version\" | \", by T. Granlund et al.\" | newline\n"
"	       else \"\"\n"
"	       )\n"
"     	  |    (\n"
"	       if version#\"mpir version\" =!= \"not present\"\n"
"	       then \"  MPIR \" | version#\"mpir version\" | \", by Free Software Foundation et al.\" | newline\n"
"	       else \"\"\n"
"	       )\n"
"     	  | \"  MPFR \" | version#\"mpfr version\" | \", by Free Software Foundation\" | newline\n"
"	  | \"  BLAS and LAPACK 3.0\" | \", by J. Dongarra et al.\"\n"
"	  );\n"
"\n"
"     scan(\n"
"	  { (\"factory version\", \"3.0.2\"), (\"libfac version\", \"3.0.1\") },\n"
"	  (k,v) -> if version#k < v then stderr << \"--warning: old \" << k << \" \" << version#k << \" < \" << v << endl);\n"
"\n"
"     use = x -> x;				  -- temporary, until methods.m2\n"
"\n"
"     Attributes = new MutableHashTable;\n"
"     -- values are hash tables with keys Symbol, String, Net (as symbols); replaces ReverseDictionary and PrintNames\n"
"     setAttribute = (val,attr,x) -> (\n"
"	  if Attributes#?val then Attributes#val else Attributes#val = new MutableHashTable\n"
"	  )#attr = x;\n"
"     hasAnAttribute = (val) -> Attributes#?val;\n"
"     hasAttribute = (val,attr) -> Attributes#?val and Attributes#val#?attr;\n"
"     getAttribute = (val,attr) -> Attributes#val#attr;\n"
"     getAttributes = (attr0) -> (\n"
"	  r := new MutableHashTable;\n"
"	  scan(values Attributes, tab -> scan(pairs tab, (attr,x) -> if attr === attr0 then r#x = true));\n"
"	  keys r);\n"
"     removeAttribute = (val,attr) -> (\n"
"	  a := Attributes#val;\n"
"	  remove(a,attr);\n"
"	  if #a === 0 then remove(Attributes,val);\n"
"	  );\n"
"     protect PrintNet;\n"
"     protect PrintNames;\n"
"     protect ReverseDictionary;\n"
"\n"
"     globalAssign = (s,v) -> if v =!= value s then (\n"
"	  X := class value s;\n"
"	  m := lookup(GlobalReleaseHook,X);\n"
"	  if m =!= null then m(s,value s);\n"
"	  Y := class v;\n"
"	  n := lookup(GlobalAssignHook,Y);\n"
"	  if n =!= null then n(s,v);\n"
"	  s <- v);\n"
"     globalAssignFunction = (X,x) -> (\n"
"	  if not instance(X,Symbol) then error(\"globalAssignFunction: expected a symbol: \", toString X);\n"
"	  if not hasAttribute(x,ReverseDictionary) then setAttribute(x,ReverseDictionary,X);\n"
"	  use x;\n"
"	  );\n"
"     globalReleaseFunction = (X,x) -> (\n"
"	  if not instance(X,Symbol) then error(\"globalAssignFunction: expected a symbol: \", toString X);\n"
"	  if hasAttribute(x,ReverseDictionary)\n"
"	  and getAttribute(x,ReverseDictionary) === X\n"
"	  then removeAttribute(x,ReverseDictionary)\n"
"	  );\n"
"     globalAssignment = X -> (\n"
"	  if instance(X, VisibleList) then apply(X,globalAssignment)\n"
"	  else if instance(X,Type) then (\n"
"	       X.GlobalAssignHook = globalAssignFunction; \n"
"	       X.GlobalReleaseHook = globalReleaseFunction;\n"
"	       )\n"
"	  else error \"expected a type\";\n"
"	  );\n"
"     globalAssignment {Type,Function,GlobalDictionary,Manipulator};\n"
"     scan((symbol true,symbol false,symbol stdio, symbol stderr), sym -> globalAssignFunction(sym, value sym));\n"
"     scan(dictionaryPath, dict -> (\n"
"	       scan(pairs dict, (nm,sym) -> (\n"
"			 x := value sym;\n"
"			 f := lookup(GlobalAssignHook, class x);\n"
"			 if f =!= null then f(sym,x);\n"
"			 ))));\n"
"     applicationDirectorySuffix = () -> (\n"
"	  if version#\"operating system\" === \"MacOS\" then \"Library/Application Support/Macaulay2/\" else \".Macaulay2/\"\n"
"	  );\n"
"     applicationDirectory = () -> (\n"
"	  getenv \"HOME\" | \"/\" |\n"
"	  if instance(applicationDirectorySuffix, Function)\n"
"	  then applicationDirectorySuffix()\n"
"	  else applicationDirectorySuffix\n"
"	  );\n"
"\n"
"\n"
"     initlayout := () -> (\n"
"	  if regex(\".*/\",\"/aa/bb\") =!= {(0, 4)}\n"
"	  or regex(\"a|b\",\"a\") =!= {(0,1)}\n"
"	  or regex(\"^a+$\",\" \\naaa\\n \") =!= {(2,3)}\n"
"	  or replaceStrings(\"a\",\"b\",\"-a-a-\") =!= \"-b-b-\"\n"
"	  or regex(\"$a\",\"$a\") =!= null\n"
"	  or regex(\".*\",\"a\\nb\") =!= {(0, 1)}\n"
"	  or select(\"a+\",\"aaa aaaa\") =!= {\"aaa\",\"aaaa\"}\n"
"	  then error \"regex regular expression library not working\";\n"
"	  t := hashTable {\n"
"	       \"exec\" => \"${exec_prefix}/\",\n"
"	       \"common\" => \"${prefix}/\",\n"
"	       \"bin\" => \"${exec_prefix}/bin/\",\n"
"	       \"lib\" => \"${exec_prefix}/lib/\",\n"
"	       \"info\" => \"${prefix}/share/info/\",\n"
"	       \"data\" => \"${prefix}/share/\",\n"
"	       \"man\" => \"${prefix}/share/man/\",\n"
"	       \"emacs\" => \"${prefix}/share/emacs/site-lisp/\",\n"
"	       \"doc\" => \"${prefix}/share/doc/Macaulay2/\",\n"
"	       \"packages\" => \"${prefix}/share/Macaulay2/\",\n"
"	       \"docdir\" => \"${prefix}/share/doc/Macaulay2/\",\n"
"	       \"libraries\" => \"${exec_prefix}/lib/Macaulay2/x86_64-Linux-Ubuntu-11.10/lib/\",\n"
"	       \"programs\" => \"${exec_prefix}/libexec/Macaulay2/x86_64-Linux-Ubuntu-11.10/\",\n"
"	       \"program licenses\" => \"${exec_prefix}/libexec/Macaulay2/program-licenses/\",\n"
"	       \"package\" => \"${prefix}/share/Macaulay2/PKG/\",\n"
"	       \"packagedoc\" => \"${prefix}/share/doc/Macaulay2/PKG/\",\n"
"	       \"packageimages\" => \"${prefix}/share/doc/Macaulay2/PKG/images/\",\n"
"	       \"packagetests\" => \"${prefix}/share/doc/Macaulay2/PKG/tests/\",\n"
"	       \"packagehtml\" => \"${prefix}/share/doc/Macaulay2/PKG/html/\",\n"
"	       \"packagecache\" => \"${exec_prefix}/lib/Macaulay2/x86_64-Linux-Ubuntu-11.10/PKG/cache/\",\n"
"	       \"packagelib\" => \"${exec_prefix}/lib/Macaulay2/x86_64-Linux-Ubuntu-11.10/PKG/\",\n"
"	       \"packageexampleoutput\" => \"${prefix}/share/doc/Macaulay2/PKG/example-output/\"\n"
"	       };\n"
"	  Layout = hashTable {\n"
"	       1 => applyValues( t, x -> replaceStrings(\"^\\\\$\\\\{(pre_)?(exec_)?prefix\\\\}/\",\"\",x)),\n"
"	       2 => applyValues( t, x -> \n"
"		    replaceStrings(\"^\\\\$\\\\{(pre_)?prefix\\\\}\",\"common\",	    -- as in configure.ac\n"
"			 replaceStrings(\"^\\\\$\\\\{(pre_)?exec_prefix\\\\}\",version#\"machine\", -- as in configure.ac\n"
"			      x)))};\n"
"	  );\n"
"     initlayout();\n"
"     )\n"
"\n"
"dir  := s -> ( m := regex(\".*/\",s); if m === null or 0 === #m then \"./\" else substring(m#0#0,m#0#1-1,s))\n"
"base := s -> ( m := regex(\".*/\",s); if m === null or 0 === #m then s    else substring(m#0#1,      s))\n"
"initcurrentlayout := () -> (\n"
"     stderr << \"Initcurrentlayout...\" << endl;\n"
"     issuffix := (s,t) -> t =!= null and s === substring(t,-#s);\n"
"     bindir := dir exe | \"/\";\n"
"     stderr << bindir << endl;\n"
"     stderr << Layout#2#\"bin\" << endl;\n"
"     stderr << Layout#1#\"bin\" << endl;\n"
"     currentLayout = (\n"
"	  if issuffix(Layout#2#\"bin\",bindir) then Layout#2 else\n"
"	  if issuffix(Layout#1#\"bin\",bindir) then Layout#1\n"
"	  );\n"
"     stderr << currentLayout << endl;\n"
"     prefixDirectory = if currentLayout =!= null then substring(bindir,0,#bindir-#currentLayout#\"bin\");\n"
"     stderr << prefixDirectory << endl;\n"
"     if notify then stderr << \"--prefixDirectory = \" << prefixDirectory << endl;\n"
"     if readlink exe =!= null then (\n"
"	  exe2 := concatPath(realpath dir exe, readlink exe);\n"
"	  bindir2 := dir exe2 | \"/\";\n"
"	  currentLayout2 := (\n"
"	       if issuffix(Layout#2#\"bin\",bindir2) then Layout#2 else\n"
"	       if issuffix(Layout#1#\"bin\",bindir2) then Layout#1\n"
"	       );\n"
"	  );\n"
"     prefixDirectory2 := if currentLayout2 =!= null then substring(bindir2,0,#bindir2-#currentLayout2#\"bin\");\n"
"     if prefixDirectory2 =!= null\n"
"	and isDirectory(prefixDirectory2|currentLayout2#\"packages\")\n"
"	and (currentLayout === null or not isDirectory(prefixDirectory|currentLayout#\"packages\"))\n"
"     then (\n"
"	  prefixDirectory = prefixDirectory2;\n"
"	  currentLayout = currentLayout2;\n"
"	  );\n"
"     stA := \"StagingArea/\";\n"
"     topBuilddir = (\n"
"	  if issuffix(stA,prefixDirectory) then substring(prefixDirectory,0,#prefixDirectory-#stA)\n"
"	  else\n"
"	  if issuffix(stA,prefixDirectory2) then substring(prefixDirectory2,0,#prefixDirectory2-#stA));\n"
"     if notify then stderr << \"--topBuilddir = \" << topBuilddir << endl;\n"
"     topSrcdir = if topBuilddir =!= null and fileExists(topBuilddir|\"srcdir\") then (\n"
"	  sdir := first lines get(topBuilddir|\"srcdir\");\n"
"	  minimizeFilename concatPath(topBuilddir,sdir));\n"
"     if notify then stderr << \"--topSrcdir = \" << topSrcdir << endl;\n"
"     )\n"
"\n"
"prefixDirectory = null					    -- prefix directory, after installation, e.g., \"/usr/local/\"\n"
"encapDirectory = null	   -- encap directory, after installation, if present, e.g., \"/usr/local/encap/Macaulay2-0.9.5/\"\n"
"\n"
"fullCopyright := false\n"
"matchpart := (pat,i,s) -> substring_((regex(pat, s))#i) s\n"
"notdir := s -> matchpart(\"[^/]*$\",0,s)\n"
"noloaddata := false\n"
"nosetup := false\n"
"noinitfile = false\n"
"interpreter := commandInterpreter\n"
"\n"
"{*\n"
"getRealPath := fn -> (					    -- use this later if realpath doesn't work\n"
"     local s;\n"
"     while ( s = readlink fn; s =!= null ) do fn = if isAbsolutePath s then s else minimizeFilename(fn|\"/../\"|s);\n"
"     fn)\n"
"*}\n"
"\n"
"pathsearch := e -> (\n"
"     if not isFixedExecPath e then (\n"
"	  -- we search the path, but we don't do it the same way execvp does, too bad.\n"
"	  PATH := separate(\":\",if \"\" =!= getenv \"PATH\" then getenv \"PATH\" else \".:/bin:/usr/bin\");\n"
"	  PATH = apply(PATH, x -> if x === \"\" then \".\" else x);\n"
"	  scan(PATH, p -> if fileExists (p|\"/\"|e) then (e = p|\"/\"|e; break));\n"
"	  );\n"
"     e)\n"
"\n"
"phase := 1\n"
"\n"
"silence := arg -> null\n"
"notyeterr := arg -> error(\"command line option \", arg, \" not re-implemented yet\")\n"
"notyet := arg -> if phase == 1 then (\n"
"     << \"warning: command line option \" << arg << \" not re-implemented yet\" << newline << flush;\n"
"     )\n"
"obsolete := arg -> error (\"warning: command line option \", arg, \" is obsolete\")\n"
"progname := notdir commandLine#0\n"
"\n"
"usage := arg -> (\n"
"     << \"usage:\"             << newline\n"
"     << \"    \" << progname << \" [option ...] [file ...]\" << newline\n"
"     << \"options:\"  << newline\n"
"     << \"    --help             print this brief help message and exit\" << newline\n"
"     << \"    --no-backtrace     print no backtrace after error\" << newline\n"
"     << \"    --copyright        display full copyright messasge\" << newline\n"
"     << \"    --no-debug         do not enter debugger upon error\" << newline\n"
"     << \"    --debug n          debug warning message n by signalling an error\" << newline\n"
"     << \"    --int              don't handle interrupts\" << newline -- handled by M2lib.c\n"
"     << \"    --notify           notify when loading files during initialization\" << newline\n"
"     << \"                       and when evaluating command line arguments\" << newline\n"
"     << \"    --no-prompts       print no input prompts\" << newline;\n"
"     << \"    --no-readline      don't use readline\" << newline;\n"
"     << \"    --no-setup         don't try to load setup.m2 or to loaddata\" << newline\n"
"     << \"    --no-personality   don't set the personality and re-exec M2 (linux only)\" << newline\n"
"     << \"    --prefix DIR       set prefixDirectory\" << newline\n"
"     << \"    --print-width n    set printWidth=n (the default is the window width)\" << newline\n"
"     << \"    --restarted        used internally to indicate this is a restart\" << newline\n"
"     << \"    --script           as first argument, interpret second argument as name of a script\" << newline\n"
"     << \"                       implies --stop, --no-debug, --silent and -q\" << newline\n"
"     << \"                       see scriptCommandLine\" << newline\n"
"     << \"    --silent           no startup banner\" << newline\n"
"     << \"    --stop             exit on error\" << newline\n"
"     << \"    --texmacs          TeXmacs session mode\" << newline\n"
"     << \"    --version          print version number and exit\" << newline\n"
"     << \"    -q                 don't load user's init.m2 file or use packages in home directory\" << newline\n"
"     << \"    -E '...'           evaluate expression '...' before initialization\" << newline\n"
"     << \"    -e '...'           evaluate expression '...' after initialization\" << newline\n"
"     << \"    --top-srcdir '...' add top source or build tree '...' to initial path\" << newline\n"
"     << \"environment:\"       << newline\n"
"     << \"    EDITOR             default text editor\" << newline\n"
"     ;)\n"
"\n"
"\n"
"tryLoad := (ofn,fn) -> if fileExists fn then (\n"
"     filetime := fileTime fn;\n"
"     r := simpleLoad fn;\n"
"     markLoaded(fn,ofn,notify,filetime);\n"
"     true) else false\n"
"\n"
"showMaps := () -> (\n"
"     if version#\"operating system\" === \"SunOS\" then (\n"
"	  stack lines get (\"!/usr/bin/pmap \"|processID())\n"
"	  )\n"
"     else if version#\"operating system\" === \"Linux\" and fileExists(\"/proc/\"|toString processID()|\"/maps\") then (\n"
"	  stack lines get(\"/proc/\"|toString processID()|\"/maps\")\n"
"	  )\n"
"     else \"memory maps not available\"\n"
"     )\n"
"\n"
"\n"
"argno := 1\n"
"\n"
"action := hashTable {\n"
"     \"-h\" => arg -> (usage(); exit 0),\n"
"     \"-mpwprompt\" => notyeterr,\n"
"     \"-n\" => obsolete,\n"
"     \"-q\" => arg -> noinitfile = true,\n"
"     \"-s\" => obsolete,\n"
"     \"-silent\" => obsolete,\n"
"     \"-tty\" => notyet,\n"
"     \"--copyright\" => arg -> if phase == 1 then fullCopyright = true,\n"
"     \"--help\" => arg -> (usage(); exit 0),\n"
"     \"--int\" => arg -> arg,\n"
"     \"--no-backtrace\" => arg -> if phase == 1 then backtrace = false,\n"
"     \"--no-debug\" => arg -> debuggingMode = false,\n"
"     \"--no-loaddata\" => arg -> if phase == 1 then noloaddata = true,\n"
"     \"--no-personality\" => arg -> arg,\n"
"     \"--no-prompts\" => arg -> if phase == 3 then noPrompts(),\n"
"     \"--no-readline\" => arg -> arg,			    -- handled in d/stdio.d\n"
"     \"--no-setup\" => arg -> if phase == 1 then noloaddata = nosetup = true,\n"
"     \"--notify\" => arg -> if phase <= 2 then notify = true,\n"
"     \"--no-tty\" => arg -> arg,			    -- handled in d/stdio.d\n"
"     \"--script\" => arg -> error \"script option should be first argument, of two\",\n"
"     \"--silent\" => arg -> nobanner = true,\n"
"     \"--stop\" => arg -> (if phase == 1 then stopIfError = true; debuggingMode = false;), -- see also M2lib.c and tokens.d\n"
"     \"--restarted\" => arg -> restarted = true,\n"
"     \"--texmacs\" => arg -> (\n"
"	  if phase == 1 then (\n"
"	       topLevelMode = global TeXmacs;\n"
"	       printWidth = 80;\n"
"	       )\n"
"	  else if phase == 3 then (\n"
"	       topLevelMode = global TeXmacs;\n"
"	       printWidth = 80;\n"
"	       )\n"
"	  else if phase == 4 then (\n"
"	       texmacsmode = true;\n"
"	       topLevelMode = TeXmacs;\n"
"	       addEndFunction(() -> if texmacsmode then (\n"
"			 if restarting \n"
"			 then stderr << \"Macaulay2 restarting...\" << endl << endl << flush\n"
"			 else (\n"
"			      stderr << \"Macaulay2 exiting\" << flush;\n"
"			      << TeXmacsEnd << endl << flush)));\n"
"	       )\n"
"	  ),\n"
"     \"--version\" => arg -> ( << version#\"VERSION\" << newline; exit 0; )\n"
"     };\n"
"\n"
"valueNotify := arg -> (\n"
"     if notify then stderr << \"--evaluating command line argument \" << argno << \": \" << format arg << endl;\n"
"     value arg)\n"
"\n"
"initialPath := {}\n"
"\n"
"action2 := hashTable {\n"
"     \"--srcdir\" => arg -> if phase == 2 then (\n"
"	  if not match(\"/$\",arg) then arg = arg|\"/\";\n"
"	  srcdirs = append(srcdirs,arg);\n"
"	  initialPath = join(initialPath,select({arg|\"Macaulay2/m2/\",arg|\"Macaulay2/packages/\"},isDirectory));\n"
"	  ),\n"
"     \"-E\" => arg -> if phase == 3 then valueNotify arg,\n"
"     \"-e\" => arg -> if phase == 4 then valueNotify arg,\n"
"     \"--print-width\" => arg -> if phase == 3 then printWidth = value arg,\n"
"     \"--debug\" => arg -> debugWarningHashcode = value arg,\n"
"     \"--prefix\" => arg -> if phase == 1 or phase == 3 then (\n"
"	  if not match(\"/$\",arg) then arg = arg | \"/\";\n"
"	  prefixDirectory = arg;\n"
"	  )\n"
"     }\n"
"\n"
"scriptCommandLine = {}\n"
"\n"
"processCommandLineOptions := phase0 -> (			    -- 3 passes\n"
"     phase = phase0;\n"
"     argno = 1;\n"
"     if commandLine#?1 and commandLine#1 == \"--script\" then (\n"
"	  if phase <= 2 then (\n"
"	       clearEcho stdio;\n"
"	       debuggingMode = false;\n"
"	       stopIfError = noinitfile = nobanner = true;\n"
"	       )\n"
"	  else if phase == 4 then (\n"
"	       if not commandLine#?2 then error \"script file name missing\";\n"
"	       arg := commandLine#2;\n"
"	       scriptCommandLine = drop(commandLine,2);\n"
"	       if instance(value global load, Function) then load arg else simpleLoad arg;\n"
"	       exit 0))\n"
"     else (\n"
"	  if notify then stderr << \"--phase \" << phase << endl;\n"
"	  while argno < #commandLine do (\n"
"	       arg = commandLine#argno;\n"
"	       if action#?arg then action#arg arg\n"
"	       else if action2#?arg then (\n"
"		    if argno < #commandLine + 1\n"
"		    then action2#arg commandLine#(argno = argno + 1)\n"
"		    else error(\"command line option \", arg, \" missing argument\")\n"
"		    )\n"
"	       else if arg#0 == \"-\" then (\n"
"		    stderr << \"error: unrecognized command line option: \" << arg << endl;\n"
"		    usage();\n"
"		    exit 1;\n"
"		    )\n"
"	       else if phase == 4 then (\n"
"		    if not isStablePath arg and instance(load, Function) then load arg\n"
"		    else simpleLoad arg;\n"
"		    );\n"
"	       argno = argno+1;\n"
"	       );\n"
"	  ))\n"
"\n"
"if firstTime then processCommandLineOptions 1\n"
"\n"
"exe = minimizeFilename (\n"
"     {*\n"
"     -- this can be a reliable way to get the executable in linux\n"
"     -- but we don't want to use it because we don't want to chase symbolic links and it does that for us\n"
"     processExe := \"/proc/self/exe\";\n"
"     if fileExists processExe and readlink processExe =!= null then readlink processExe\n"
"     else \n"
"     *}\n"
"     if isAbsolutePath commandLine#0 then commandLine#0 else\n"
"     if isStablePath commandLine#0 then concatenate(currentDirectory()|commandLine#0)\n"
"     else pathsearch commandLine#0)\n"
"if not isAbsolutePath exe then exe = currentDirectory() | exe ;\n"
"exe = concatenate(realpath dir exe, base exe)\n"
"if notify then stderr << \"--executable = \" << exe << endl\n"
"\n"
"describePath := () -> (\n"
"     if #path == 0 then stderr << \"--file search path empty\" << endl\n"
"     else (\n"
"	  stderr << \"--file search path directories:\" << endl;\n"
"	  for d in path do (\n"
"	       stderr << \"--    \" << d << endl;\n"
"	       );\n"
"	  )\n"
"     )\n"
"\n"
"loadSetup := () -> (\n"
"     if notify then describePath();\n"
"     for d in path do (\n"
"	  fn := minimizeFilename(d|\"setup.m2\");\n"
"	  if tryLoad(\"setup.m2\", fn) then return;\n"
"	  );\n"
"     if not notify then describePath();\n"
"     error \"can't load setup.m2\"\n"
"     )\n"
"\n"
"\n"
"if firstTime and not nobanner then (\n"
"     if topLevelMode === TeXmacs then stderr << TeXmacsBegin << \"verbatim:\";\n"
"     stderr << (if fullCopyright then copyright else first separate copyright) << newline << flush;\n"
"     if topLevelMode === TeXmacs then stderr << TeXmacsEnd << flush;\n"
")\n"
"\n"
"scan(commandLine, arg -> if arg === \"-q\" then noinitfile = true)\n"
"homeDirectory = getenv \"HOME\" | \"/\"\n"
"\n"
"initcurrentlayout()\n"
"path = (x -> select(x, i -> i =!= null)) deepSplice {\n"
"	  if prefixDirectory =!= null then (\n"
"	       if topBuilddir =!= null then (\n"
"		    if topSrcdir =!= null then (\n"
"		    	 topSrcdir|\"Macaulay2/m2/\",\n"
"		    	 topSrcdir|\"Macaulay2/packages/\"\n"
"			 ),\n"
"		    topBuilddir|\"Macaulay2/m2/\",\n"
"		    topBuilddir|\"Macaulay2/packages/\"\n"
"		    ),\n"
"	       prefixDirectory | replaceStrings(\"PKG\",\"Core\",currentLayout#\"package\"),\n"
"	       prefixDirectory | currentLayout#\"packages\"\n"
"	       )\n"
"	  }\n"
"packagepath = (x -> select(x, i -> i =!= null)) deepSplice {\n"
"	  if prefixDirectory =!= null then (\n"
"	       if topBuilddir =!= null then (\n"
"		    if topSrcdir =!= null then (\n"
"		    	 topSrcdir|\"Macaulay2/packages/\"\n"
"			 ),\n"
"		    topBuilddir|\"Macaulay2/packages/\"\n"
"		    ),\n"
"	       prefixDirectory | currentLayout#\"packages\"\n"
"	       )\n"
"	  }\n"
"\n"
"if firstTime then normalPrompts()\n"
"\n"
"printWidth = fileWidth stdio\n"
"\n"
"processCommandLineOptions 2				    -- just for path to core files and packages\n"
"\n"
"path = join(initialPath, path)\n"
"\n"
"(loadDepth = 1; if firstTime and not nosetup then loadSetup(); loadDepth = 0)\n"
"\n"
"-- remove the Core private dictionary -- it was added by \"debug\" above\n"
"-- and install a local way to use private global symbols\n"
"local core\n"
"if not nosetup then (\n"
"     dictionaryPath = select(dictionaryPath, d -> d =!= Core#\"private dictionary\");\n"
"     core = nm -> value Core#\"private dictionary\"#nm;\n"
"     ) else (\n"
"     core = nm -> value getGlobalSymbol nm\n"
"     )\n"
"\n"
"(loadDepth = 3; processCommandLineOptions 3; loadDepth = 0)\n"
"(core \"runStartFunctions\")()\n"
"\n"
"if class Core =!= Symbol and not core \"noinitfile\" then (\n"
"     -- the location of init.m2 is documented in the node \"initialization file\"\n"
"     tryLoad (\"init.m2\", applicationDirectory() | \"init.m2\");\n"
"     );\n"
"\n"
"loadDepth = 0;\n"
"( loadDepth = 3;\n"
"     processCommandLineOptions 4;\n"
"     n := 0;\n"
"     errorDepth = loadDepth = 3;\n"
"     n = interpreter();\n"
"     if class n === ZZ and 0 <= n and n < 128 then exit n;\n"
"     if n === null then exit 0;\n"
"     debuggingMode = false;\n"
"     stopIfError = true;\n"
"     stderr << \"error: can't interpret return value as an exit code\" << endl;\n"
"     exit 1;\n"
"     )\n"
"-- Local Variables:\n"
"-- compile-command: \"make -C $M2BUILDDIR/Macaulay2/d startup.m2 all && make -C $M2BUILDDIR/Macaulay2/bin all \"\n"
"-- End:\n"
;
char startupFile  [] =
"/home/gfurnish/M2/gfurnish/scons/M2/BUILD/normal/../../Macaulay2/d/startup.m2.in"
;
