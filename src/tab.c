/* 16sep03abu
 * (c) Software Lab. Alexander Burger
 */

#include "pico.h"

typedef struct symInit {fun code; char *name;} symInit;

static symInit Symbols[] = {
   {doAbs, "abs"},
   {doAdd, "+"},
   {doAll, "all"},
   {doAnd, "and"},
   {doAny, "any"},
   {doAppend, "append"},
   {doApply, "apply"},
   {doArg, "arg"},
   {doArgs, "args"},
   {doArgv, "argv"},
   {doAsoq, "asoq"},
   {doAssoc, "assoc"},
   {doAt, "at"},
   {doAtom, "atom"},
   {doBegin, "begin"},
   {doBind, "bind"},
   {doBitAnd, "&"},
   {doBitOr, "|"},
   {doBitQ, "bit?"},
   {doBool, "bool"},
   {doBox, "box"},
   {doBreak, "!"},
   {doBye, "bye"},
   {doCaaar, "caaar"},
   {doCaadr, "caadr"},
   {doCaar, "caar"},
   {doCadar, "cadar"},
   {doCadddr, "cadddr"},
   {doCaddr, "caddr"},
   {doCadr, "cadr"},
   {doCall, "call"},
   {doCar, "car"},
   {doCase, "case"},
   {doCatch, "catch"},
   {doCd, "cd"},
   {doCdaar, "cdaar"},
   {doCdadr, "cdadr"},
   {doCdar, "cdar"},
   {doCddar, "cddar"},
   {doCddddr, "cddddr"},
   {doCdddr, "cdddr"},
   {doCddr, "cddr"},
   {doCdr, "cdr"},
   {doChar, "char"},
   {doChain, "chain"},
   {doChop, "chop"},
   {doCirc, "circ"},
   {doClip, "clip"},
   {doClose, "close"},
   {doCnt, "cnt"},
   {doCol, ":"},
   {doCommit, "commit"},
   {doCon, "con"},
   {doConc, "conc"},
   {doCond, "cond"},
   {doConnect, "connect"},
   {doCons, "cons"},
   {doCopy, "copy"},
   {doCtl, "ctl"},
   {doCtty, "ctty"},
   {doCut, "cut"},
   {doDate, "date"},
   {doDbck, "dbck"},
   {doDe, "de"},
   {doDec, "dec"},
   {doDef, "def"},
   {doDefault, "default"},
   {doDelete, "delete"},
   {doDelq, "delq"},
   {doDiv, "/"},
   {doDm, "dm"},
   {doDo, "do"},
   {doE, "e"},
   {doEcho, "echo"},
   {doEnv, "env"},
   {doEq, "=="},
   {doEqual, "="},
   {doEqual0, "=0"},
   {doEqualT, "=T"},
   {doEval, "eval"},
   {doExtern, "extern"},
   {doExtQ, "ext?"},
   {doExtra, "extra"},
   {doFill, "fill"},
   {doFilter, "filter"},
   {doFind, "find"},
   {doFlush, "flush"},
   {doFold, "fold"},
   {doFor, "for"},
   {doFork, "fork"},
   {doFormat, "format"},
   {doFrom, "from"},
   {doFunQ, "fun?"},
   {doGc, "gc"},
   {doGe, ">="},
   {doGet, "get"},
   {doGetl, "getl"},
   {doGt, ">"},
   {doGt0, "gt0"},
   {doHead, "head"},
   {doHeap, "heap"},
   {doHear, "hear"},
   {doHide, "===="},
   {doHost, "host"},
   {doIdx, "idx"},
   {doIf, "if"},
   {doIfn, "ifn"},
   {doIn, "in"},
   {doInc, "inc"},
   {doIndex, "index"},
   {doInfo, "info"},
   {doIntern, "intern"},
   {doIsa, "isa"},
   {doJob, "job"},
   {doKey, "key"},
   {doKill, "kill"},
   {doLast, "last"},
   {doLe, "<="},
   {doLength, "length"},
   {doLet, "let"},
   {doLine, "line"},
   {doLines, "lines"},
   {doLink, "link"},
   {doList, "list"},
   {doListen, "listen"},
   {doLstQ, "lst?"},
   {doLoad, "load"},
   {doLock, "lock"},
   {doLookup, "->"},
   {doLoop, "loop"},
   {doLowQ, "low?"},
   {doLowc, "lowc"},
   {doLt, "<"},
   {doLt0, "lt0"},
   {doMade, "made"},
   {doMake, "make"},
   {doMap, "map"},
   {doMapc, "mapc"},
   {doMapcan, "mapcan"},
   {doMapcar, "mapcar"},
   {doMapcon, "mapcon"},
   {doMaplist, "maplist"},
   {doMaps, "maps"},
   {doMark, "mark"},
   {doMatch, "match"},
   {doMax, "max"},
   {doMaxi, "maxi"},
   {doMember, "member"},
   {doMemq, "memq"},
   {doMeta, "meta"},
   {doMethod, "method"},
   {doMin, "min"},
   {doMini, "mini"},
   {doMix, "mix"},
   {doMmeq, "mmeq"},
   {doMul, "*"},
   {doMulDiv, "*/"},
   {doNand, "nand"},
   {doNEq, "n=="},
   {doNEq0, "n0"},
   {doNEqT, "nT"},
   {doNEqual, "<>"},
   {doNeed, "need"},
   {doNew, "new"},
   {doNext, "next"},
   {doNil, "nil"},
   {doNor, "nor"},
   {doNot, "not"},
   {doNth, "nth"},
   {doNumQ, "num?"},
   {doOff, "off"},
   {doOffset, "offset"},
   {doOn, "on"},
   {doOpen, "open"},
   {doOr, "or"},
   {doOut, "out"},
   {doPack, "pack"},
   {doPair, "pair"},
   {doPass, "pass"},
   {doPatQ, "pat?"},
   {doPeek, "peek"},
   {doPick, "pick"},
   {doPool, "pool"},
   {doPop, "pop"},
   {doPort, "port"},
   {doPr, "pr"},
   {doPreQ, "pre?"},
   {doPrin, "prin"},
   {doPrinl, "prinl"},
   {doPrint, "print"},
   {doPrintln, "println"},
   {doPrintsp, "printsp"},
   {doProg, "prog"},
   {doProg1, "prog1"},
   {doProg2, "prog2"},
   {doProp, "prop"},
   {doPropCol, "::"},
   {doProve, "prove"},
   {doPush, "push"},
   {doPut, "put"},
   {doPutl, "putl"},
   {doQueue, "queue"},
   {doQuit, "quit"},
   {doRand, "rand"},
   {doRank, "rank"},
   {doRd, "rd"},
   {doRead, "read"},
   {doRem, "%"},
   {doReplace, "replace"},
   {doRest, "rest"},
   {doReverse, "reverse"},
   {doRewind, "rewind"},
   {doRollback, "rollback"},
   {doRot, "rot"},
   {doRun, "run"},
   {doSeed, "seed"},
   {doSeek, "seek"},
   {doSend, "send"},
   {doSeq, "seq"},
   {doSet, "set"},
   {doSetCol, "=:"},
   {doSetq, "setq"},
   {doShift, ">>"},
   {doSize, "size"},
   {doSkip, "skip"},
   {doSort, "sort"},
   {doSpace, "space"},
   {doSplit, "split"},
   {doSpQ, "sp?"},
   {doSqrt, "sqrt"},
   {doStk, "stk"},
   {doStr, "str"},
   {doStrip, "strip"},
   {doStrQ, "str?"},
   {doSub, "-"},
   {doSubQ, "sub?"},
   {doSum, "sum"},
   {doSuper, "super"},
   {doSym, "sym"},
   {doSymQ, "sym?"},
   {doSync, "sync"},
   {doSys, "sys"},
   {doT, "t"},
   {doTail, "tail"},
   {doTell, "tell"},
   {doThrow, "throw"},
   {doTick, "tick"},
   {doTill, "till"},
   {doTime, "time"},
   {doTouch, "touch"},
   {doTrace, "$"},
   {doTrim, "trim"},
   {doType, "type"},
   {doUnify, "unify"},
   {doUnless, "unless"},
   {doUntil, "until"},
   {doUppQ, "upp?"},
   {doUppc, "uppc"},
   {doUse, "use"},
   {doVal, "val"},
   {doWait, "wait"},
   {doWhen, "when"},
   {doWhile, "while"},
   {doWhilst, "whilst"},
   {doWipe, "wipe"},
   {doWith, "with"},
   {doXchg, "xchg"},
   {doXor, "xor"},
   {doZap, "zap"},
   {doZero, "zero"},
};

any initSym(any v, char *s) {
   any x, *h;

   h = Intern + hash(x = mkName((byte*)s));
   x = consSym(v,x);
   *h = cons(x,*h);
   return x;
}

void initSymbols(void) {
   int i;

   Nil = symPtr(Avail),  Avail = Avail->cdr->cdr;  // Allocate 2 cells for NIL
   val(Nil) = tail(Nil) = val(Nil+1) = tail(Nil+1) = Nil;
   Zero = box(0);
   for (i = 0; i < HASH; ++i)
      Intern[i] = Transient[i] = Extern[i] = Nil;
   DB    = initSym(Nil, "*DB");
   Up    = initSym(Nil, "^");
   At    = initSym(Nil, "@");
   At2   = initSym(Nil, "@@");
   At3   = initSym(Nil, "@@@");
   This  = initSym(Nil, "This");
   Meth  = initSym(box(num(doMeth)), "meth");
   Quote = initSym(box(num(doQuote)), "quote");
   T     = initSym(Nil, "T"),  val(T) = T;  // Last protected symbol

   Dbg   = initSym(T, "*Dbg");
   Pid   = initSym(boxCnt(getpid()), "*Pid");
   Scl   = initSym(Zero, "*Scl");
   Class = initSym(Nil, "*Class");
   Key   = initSym(Nil, "*Key");
   Led   = initSym(Nil, "*Led");
   Err   = initSym(Nil, "*Err");
   Msg   = initSym(Nil, "*Msg");
   Adr   = initSym(Nil, "*Adr");
   Bye   = initSym(Nil, "*Bye");

   val(DB) = DbVal = extSym(consStr(DbTail = box('1')));
   Extern['1'] = cons(DbVal, Nil);

   for (i = 0; i < (int)(sizeof(Symbols)/sizeof(symInit)); ++i)
      initSym(box(num(Symbols[i].code)), Symbols[i].name);
}
