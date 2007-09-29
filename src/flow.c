/* 20sep07abu
 * (c) Software Lab. Alexander Burger
 */

#include "pico.h"

static void redefMsg(any x, any y) {
   outFile *oSave = OutFile;
   FILE *stdSave = StdOut;

   OutFile = NULL,  StdOut = stderr;
   outString("# ");
   print(x);
   if (y)
      space(), print(y);
   outString(" redefined\n");
   OutFile = oSave,  StdOut = stdSave;
}

static void putSrc(any s, any k) {
   if (!isNil(val(Dbg)) && InFile && InFile->name) {
      cell c1;

      Push(c1, boxCnt(InFile->src));
      put(s, k, cons(data(c1), mkStr(InFile->name)));
      drop(c1);
   }
}

static void redefine(any ex, any s, any x) {
   NeedSym(ex,s);
   CheckVar(ex,s);
   if (!isNil(val(s))  &&  s != val(s)  &&  !equal(x,val(s)))
      redefMsg(s, NULL);
   val(s) = x;
   putSrc(s, Dbg);
}

// (quote . any) -> any
any doQuote(any x) {return cdr(x);}

// (as 'any1 . any2) -> any2 | NIL
any doAs(any x) {
   x = cdr(x);
   if (isNil(EVAL(car(x))))
      return Nil;
   return cdr(x);
}

// (pid 'pid|lst . exe) -> any
any doPid(any x) {
   any y;

   x = cdr(x);
   if (!isCell(y = EVAL(car(x))))
      return equal(y, val(Pid))? EVAL(cdr(x)) : Nil;
   do
      if (equal(car(y), val(Pid)))
         return EVAL(cdr(x));
   while (isCell(y = cdr(y)));
   return Nil;
}

// (lit 'any) -> any
any doLit(any x) {
   x = cadr(x);
   if (isNum(x = EVAL(x)) || isSym(x) && x==val(x) || isCell(x) && isNum(car(x)))
      return x;
   return cons(Quote, x);
}

// (eval 'any ['cnt]) -> any
any doEval(any x) {
   cell c1;
   bindFrame *p;

   x = cdr(x),  Push(c1, EVAL(car(x))),  x = cdr(x);
   if (!isNum(x = EVAL(car(x))) || !(p = Env.bind))
      data(c1) = EVAL(data(c1));
   else {
      int cnt, n, i;
      bindFrame *q;

      for (cnt = (int)unBox(x), n = 0;;) {
         ++n;
         if (p->i <= 0) {
            if (p->i-- == 0) {
               for (i = 0;  i < p->cnt;  ++i) {
                  x = val(p->bnd[i].sym);
                  val(p->bnd[i].sym) = p->bnd[i].val;
                  p->bnd[i].val = x;
               }
               if (p->cnt  &&  p->bnd[0].sym == At  &&  !--cnt)
                  break;
            }
         }
         if (!(q = Env.bind->link))
            break;
         Env.bind->link = q->link,  q->link = p,  p = q;
      }
      Env.bind = p;
      data(c1) = EVAL(data(c1));
      for (;;) {
         if (p->i < 0) {
            if (++p->i == 0)
               for (i = p->cnt;  --i >= 0;) {
                  x = val(p->bnd[i].sym);
                  val(p->bnd[i].sym) = p->bnd[i].val;
                  p->bnd[i].val = x;
               }
         }
         if (!--n)
            break;
         q = Env.bind->link, Env.bind->link = q->link,  q->link = p,  p = q;
      }
      Env.bind = p;
   }
   return Pop(c1);
}

// (run 'any ['cnt]) -> any
any doRun(any x) {
   cell c1;
   bindFrame *p;

   x = cdr(x),  data(c1) = EVAL(car(x)),  x = cdr(x);
   if (!isNum(data(c1))) {
      Save(c1);
      if (!isNum(x = EVAL(car(x))) || !(p = Env.bind))
         data(c1) = isSym(data(c1))? val(data(c1)) : run(data(c1));
      else {
         int cnt, n, i;
         bindFrame *q;

         for (cnt = (int)unBox(x), n = 0;;) {
            ++n;
            if (p->i <= 0) {
               if (p->i-- == 0) {
                  for (i = 0;  i < p->cnt;  ++i) {
                     x = val(p->bnd[i].sym);
                     val(p->bnd[i].sym) = p->bnd[i].val;
                     p->bnd[i].val = x;
                  }
                  if (p->cnt  &&  p->bnd[0].sym==At  &&  !--cnt)
                     break;
               }
            }
            if (!(q = Env.bind->link))
               break;
            Env.bind->link = q->link,  q->link = p,  p = q;
         }
         Env.bind = p;
         data(c1) = isSym(data(c1))? val(data(c1)) : prog(data(c1));
         for (;;) {
            if (p->i < 0) {
               if (++p->i == 0)
                  for (i = p->cnt;  --i >= 0;) {
                     x = val(p->bnd[i].sym);
                     val(p->bnd[i].sym) = p->bnd[i].val;
                     p->bnd[i].val = x;
                  }
            }
            if (!--n)
               break;
            q = Env.bind->link, Env.bind->link = q->link,  q->link = p,  p = q;
         }
         Env.bind = p;
      }
      drop(c1);
   }
   return data(c1);
}

// (def 'sym 'any) -> sym
// (def 'sym 'sym 'any) -> sym
any doDef(any ex) {
   any x, y;
   cell c1, c2, c3;

   x = cdr(ex),  Push(c1, EVAL(car(x)));
   NeedSym(ex,data(c1));
   CheckVar(ex,data(c1));
   x = cdr(x),  Push(c2, EVAL(car(x)));
   if (!isCell(cdr(x))) {
      if (!equal(data(c2), y = val(data(c1)))) {
         if (!isNil(y)  &&  data(c1) != y)
            redefMsg(data(c1), NULL);
         Touch(ex,data(c1));
         val(data(c1)) = data(c2);
      }
      putSrc(data(c1), Dbg);
   }
   else {
      x = cdr(x),  Push(c3, EVAL(car(x)));
      if (!equal(data(c3), y = get(data(c1), data(c2)))) {
         if (!isNil(y))
            redefMsg(data(c1), data(c2));
         Touch(ex,data(c1));
         put(data(c1), data(c2), data(c3));
      }
      putSrc(data(c1), data(c2));
   }
   return Pop(c1);
}

// (de sym . any) -> sym
any doDe(any ex) {
   redefine(ex, cadr(ex), cddr(ex));
   return cadr(ex);
}

// (dm sym . fun) -> sym
// (dm (sym . cls) . fun) -> sym
// (dm (sym sym [. cls]) . fun) -> sym
any doDm(any ex) {
   any x, y, msg, cls;

   x = cdr(ex);
   if (!isCell(car(x)))
      msg = car(x),  cls = val(Class);
   else {
      msg = caar(x);
      cls = !isCell(cdar(x))? cdar(x) :
         get(isNil(cddar(x))? val(Class) : cddar(x), cadar(x));
   }
   if (msg != T)
      redefine(ex, msg, val(Meth));
   if (isSym(cdr(x))) {
      y = val(cdr(x));
      for (;;) {
         if (!isCell(y) || !isCell(car(y)))
            err(ex, msg, "Bad message");
         if (caar(y) == msg) {
            x = car(y);
            break;
         }
         y = cdr(y);
      }
   }
   for (y = val(cls);  isCell(y) && isCell(car(y));  y = cdr(y))
      if (caar(y) == msg) {
         if (!equal(cdr(x), cdar(y)))
            redefMsg(msg, cls);
         cdar(y) = cdr(x);
         putSrc(cls, msg);
         return msg;
      }
   if (!isCell(car(x)))
      val(cls) = cons(x, val(cls));
   else
      val(cls) = cons(cons(caar(x), cdr(x)), val(cls));
   putSrc(cls, msg);
   return msg;
}

/* Evaluate method invocation */
static any evMethod(any o, any expr, any x) {
   any y = car(expr);
   methFrame m;
   struct {  // bindFrame
      struct bindFrame *link;
      int i, cnt;
      struct {any sym; any val;} bnd[length(y)+3];
   } f;

   m.link = Env.meth;
   m.key = TheKey;
   m.cls = TheCls;
   f.link = Env.bind,  Env.bind = (bindFrame*)&f;
   f.i = sizeof(f.bnd) / (2*sizeof(any)) - 2;
   f.cnt = 1,  f.bnd[0].sym = At,  f.bnd[0].val = val(At);
   while (isCell(y)) {
      f.bnd[f.cnt].sym = car(y);
      f.bnd[f.cnt].val = EVAL(car(x));
      ++f.cnt, x = cdr(x), y = cdr(y);
   }
   if (isNil(y)) {
      while (--f.i > 0) {
         x = val(f.bnd[f.i].sym);
         val(f.bnd[f.i].sym) = f.bnd[f.i].val;
         f.bnd[f.i].val = x;
      }
      f.bnd[f.cnt].sym = This;
      f.bnd[f.cnt++].val = val(This);
      val(This) = o;
      Env.meth = &m;
      x = prog(cdr(expr));
   }
   else if (y != At) {
      f.bnd[f.cnt].sym = y,  f.bnd[f.cnt++].val = val(y),  val(y) = x;
      while (--f.i > 0) {
         x = val(f.bnd[f.i].sym);
         val(f.bnd[f.i].sym) = f.bnd[f.i].val;
         f.bnd[f.i].val = x;
      }
      f.bnd[f.cnt].sym = This,  f.bnd[f.cnt++].val = val(This),  val(This) = o;
      Env.meth = &m;
      x = prog(cdr(expr));
   }
   else {
      int n, cnt;
      cell *arg;
      cell c[n = cnt = length(x)];

      while (--n >= 0)
         Push(c[n], EVAL(car(x))),  x = cdr(x);
      while (--f.i > 0) {
         x = val(f.bnd[f.i].sym);
         val(f.bnd[f.i].sym) = f.bnd[f.i].val;
         f.bnd[f.i].val = x;
      }
      n = Env.next,  Env.next = cnt;
      arg = Env.arg,  Env.arg = c;
      f.bnd[f.cnt].sym = This;
      f.bnd[f.cnt++].val = val(This);
      val(This) = o;
      Env.meth = &m;
      x = prog(cdr(expr));
      if (cnt)
         drop(c[cnt-1]);
      Env.arg = arg,  Env.next = n;
   }
   while (--f.cnt >= 0)
      val(f.bnd[f.cnt].sym) = f.bnd[f.cnt].val;
   Env.bind = f.link;
   Env.meth = Env.meth->link;
   return x;
}

any method(any x) {
   any y, z;

   if (isCell(y = val(x))) {
      if (isCell(car(y))) {
         if (caar(y) == TheKey)
            return cdar(y);
         for (;;) {
            z = y;
            if (!isCell(y = cdr(y)))
               return NULL;
            if (!isCell(car(y)))
               break;
            if (caar(y) == TheKey) {
               cdr(z) = cdr(y),  cdr(y) = val(x),  val(x) = y;
               return cdar(y);
            }
         }
      }
      do
         if (x = method(car(TheCls = y)))
            return x;
      while (isCell(y = cdr(y)));
   }
   return NULL;
}

// (box 'any) -> sym
any doBox(any x) {
   x = cdr(x);
   return consSym(EVAL(car(x)), Nil);
}

// (new ['flg|num] ['typ ['any ..]]) -> obj
any doNew(any ex) {
   any x, y, *p;
   cell c1, c2;

   x = cdr(ex);
   Push(c1, consSym(Nil,Nil));
   if (isCell(y = EVAL(car(x))))
      val(data(c1)) = y;
   else {
      if (!isNil(y)) {
         p = Extern + hash(tail(data(c1)) = newId(isNum(y)? (int)unDig(y)/2 : 1));
         mkExt(data(c1));
         *p = cons(data(c1),*p);
      }
      x = cdr(x),  y = EVAL(car(x));
      NeedLst(ex,y);
      val(data(c1)) = y;
   }
   TheKey = T,  TheCls = Nil;
   if (y = method(data(c1)))
      evMethod(data(c1), y, cdr(x));
   else {
      Push(c2, Nil);
      while (isCell(x = cdr(x))) {
         data(c2) = EVAL(car(x)),  x = cdr(x);
         put(data(c1), data(c2), EVAL(car(x)));
      }
   }
   return Pop(c1);
}

// (type 'any) -> lst
any doType(any ex) {
   any x, y, z;

   x = cdr(ex),  x = EVAL(car(x));
   if (isSym(x)) {
      Fetch(ex,x);
      z = x = val(x);
      while (isCell(x)) {
         if (!isCell(car(x))) {
            y = x;
            while (isSym(car(x))) {
               if (!isCell(x = cdr(x)))
                  return isNil(x)? y : Nil;
               if (z == x)
                  return Nil;
            }
            return Nil;
         }
         if (z == (x = cdr(x)))
            return Nil;
      }
   }
   return Nil;
}

static bool isa(any ex, any cls, any x) {
   any z;

   z = x = val(x);
   while (isCell(x)) {
      if (!isCell(car(x))) {
         while (isSym(car(x))) {
            if (isExt(car(x)))
               return NO;
            if (cls == car(x) || isa(ex, cls, car(x)))
               return YES;
            if (!isCell(x = cdr(x)) || z == x)
               return NO;
         }
         return NO;
      }
      if (z == (x = cdr(x)))
         return NO;
   }
   return NO;
}

// (isa 'cls|typ 'any) -> obj | NIL
any doIsa(any ex) {
   any x;
   cell c1;

   x = cdr(ex),  Push(c1, EVAL(car(x)));
   x = cdr(x),  x = EVAL(car(x));
   drop(c1);
   if (isSym(x)) {
      if (isSym(data(c1))) {
         Fetch(ex,x);
         return isa(ex, data(c1), x)? x : Nil;
      }
      while (isCell(data(c1))) {
         Fetch(ex,x);
         if (!isa(ex, car(data(c1)), x))
            return Nil;
         data(c1) = cdr(data(c1));
      }
      return x;
   }
   return Nil;
}

// (method 'msg 'obj) -> fun
any doMethod(any ex) {
   any x, y;

   x = cdr(ex),  y = EVAL(car(x));
   x = cdr(x),  x = EVAL(car(x));
   Fetch(ex,x);
   TheKey = y;
   return method(x)? : Nil;
}

// (meth 'obj ..) -> any
any doMeth(any ex) {
   any x, y;
   cell c1;

   x = cdr(ex),  Push(c1, EVAL(car(x)));
   NeedSym(ex,data(c1));
   Fetch(ex,data(c1));
   for (TheKey = car(ex); ; TheKey = val(TheKey)) {
      if (!isSym(TheKey))
         err(ex, car(ex), "Bad message");
      if (isNum(val(TheKey))) {
         TheCls = Nil;
         if (y = method(data(c1))) {
            x = evMethod(data(c1), y, cdr(x));
            drop(c1);
            return x;
         }
         err(ex, TheKey, "Bad message");
      }
   }
}

// (send 'msg 'obj ['any ..]) -> any
any doSend(any ex) {
   any x, y;
   cell c1, c2;

   x = cdr(ex),  Push(c1,  EVAL(car(x)));
   NeedSym(ex,data(c1));
   x = cdr(x),  Push(c2,  EVAL(car(x)));
   NeedSym(ex,data(c2));
   Fetch(ex,data(c2));
   TheKey = data(c1),  TheCls = Nil;
   if (y = method(data(c2))) {
      x = evMethod(data(c2), y, cdr(x));
      drop(c1);
      return x;
   }
   err(ex, TheKey, "Bad message");
}

// (try 'msg 'obj ['any ..]) -> any
any doTry(any ex) {
   any x, y;
   cell c1, c2;

   x = cdr(ex),  Push(c1,  EVAL(car(x)));
   NeedSym(ex,data(c1));
   x = cdr(x),  Push(c2,  EVAL(car(x)));
   if (isSym(data(c2))) {
      if (isExt(data(c2))) {
         if (!isLife(data(c2)))
            return Nil;
         db(ex,data(c2),1);
      }
      TheKey = data(c1),  TheCls = Nil;
      if (y = method(data(c2))) {
         x = evMethod(data(c2), y, cdr(x));
         drop(c1);
         return x;
      }
   }
   drop(c1);
   return Nil;
}

// (super ['any ..]) -> any
any doSuper(any ex) {
   any x, y;
   methFrame m;

   m.key = TheKey = Env.meth->key;
   x = val(isNil(Env.meth->cls)? val(This) : car(Env.meth->cls));
   while (isCell(car(x)))
      x = cdr(x);
   while (isCell(x)) {
      if (y = method(car(TheCls = x))) {
         m.cls = TheCls;
         m.link = Env.meth,  Env.meth = &m;
         x = evExpr(y, cdr(ex));
         Env.meth = Env.meth->link;
         return x;
      }
      x = cdr(x);
   }
   err(ex, TheKey, "Bad super");
}

static any extra(any x) {
   any y;

   for (x = val(x); isCell(car(x)); x = cdr(x));
   while (isCell(x)) {
      if (x == Env.meth->cls  ||  !(y = extra(car(x)))) {
         while (isCell(x = cdr(x)))
            if (y = method(car(TheCls = x)))
               return y;
         return NULL;
      }
      if (y  &&  num(y) != 1)
         return y;
      x = cdr(x);
   }
   return (any)1;
}

// (extra ['any ..]) -> any
any doExtra(any ex) {
   any x, y;
   methFrame m;

   m.key = TheKey = Env.meth->key;
   if ((y = extra(val(This)))  &&  num(y) != 1) {
      m.cls = TheCls;
      m.link = Env.meth,  Env.meth = &m;
      x = evExpr(y, cdr(ex));
      Env.meth = Env.meth->link;
      return x;
   }
   err(ex, TheKey, "Bad extra");
}

// (with 'sym . prg) -> any
any doWith(any ex) {
   any x;
   bindFrame f;

   x = cdr(ex);
   if (isNil(x = EVAL(car(x))))
      return Nil;
   NeedSym(ex,x);
   Bind(This,f),  val(This) = x;
   x = prog(cddr(ex));
   Unbind(f);
   return x;
}

// (bind 'sym|lst . prg) -> any
any doBind(any ex) {
   any x, y;

   x = cdr(ex);
   if (isNum(y = EVAL(car(x))))
      argError(ex, y);
   if (isNil(y))
      return prog(cdr(x));
   if (isSym(y)) {
      bindFrame f;

      Bind(y,f);
      x = prog(cdr(x));
      Unbind(f);
      return x;
   }
   {
      struct {  // bindFrame
         struct bindFrame *link;
         int i, cnt;
         struct {any sym; any val;} bnd[length(y)];
      } f;

      f.link = Env.bind,  Env.bind = (bindFrame*)&f;
      f.i = f.cnt = 0;
      while (isCell(y)) {
         if (isNum(car(y)))
            argError(ex, car(y));
         if (isSym(car(y))) {
            f.bnd[f.cnt].sym = car(y);
            f.bnd[f.cnt].val = val(car(y));
         }
         else {
            f.bnd[f.cnt].sym = caar(y);
            f.bnd[f.cnt].val = val(caar(y));
            val(caar(y)) = cdar(y);
         }
         ++f.cnt,  y = cdr(y);
      }
      x = prog(cdr(x));
      while (--f.cnt >= 0)
         val(f.bnd[f.cnt].sym) = f.bnd[f.cnt].val;
      Env.bind = f.link;
      return x;
   }
}

// (job 'lst . prg) -> any
any doJob(any ex) {
   any x = cdr(ex);
   any y = EVAL(car(x));
   any z;
   cell c1;
   struct {  // bindFrame
      struct bindFrame *link;
      int i, cnt;
      struct {any sym; any val;} bnd[length(y)];
   } f;

   Push(c1,y);
   f.link = Env.bind,  Env.bind = (bindFrame*)&f;
   f.i = f.cnt = 0;
   while (isCell(y)) {
      f.bnd[f.cnt].sym = caar(y);
      f.bnd[f.cnt].val = val(caar(y));
      val(caar(y)) = cdar(y);
      ++f.cnt,  y = cdr(y);
   }
   z = prog(cdr(x));
   for (f.cnt = 0, y = Pop(c1);  isCell(y);  ++f.cnt, y = cdr(y)) {
      cdar(y) = val(caar(y));
      val(caar(y)) = f.bnd[f.cnt].val;
   }
   Env.bind = f.link;
   return z;
}

// (let sym 'any . prg) -> any
// (let (sym 'any ..) . prg) -> any
any doLet(any x) {
   any y;

   x = cdr(x);
   if (isSym(y = car(x))) {
      bindFrame f;

      x = cdr(x),  Bind(y,f),  val(y) = EVAL(car(x));
      x = prog(cdr(x));
      Unbind(f);
   }
   else {
      struct {  // bindFrame
         struct bindFrame *link;
         int i, cnt;
         struct {any sym; any val;} bnd[(length(y)+1)/2];
      } f;

      f.link = Env.bind,  Env.bind = (bindFrame*)&f;
      f.i = f.cnt = 0;
      do {
         f.bnd[f.cnt].sym = car(y);
         f.bnd[f.cnt].val = val(car(y));
         val(car(y)) = EVAL(cadr(y));
         ++f.cnt;
      } while (isCell(y = cddr(y)));
      x = prog(cdr(x));
      while (--f.cnt >= 0)
         val(f.bnd[f.cnt].sym) = f.bnd[f.cnt].val;
      Env.bind = f.link;
   }
   return x;
}

// (let? sym 'any . prg) -> any
any doLetQ(any ex) {
   any x, y, z;
   bindFrame f;

   x = cdr(ex),  y = car(x),  x = cdr(x);
   if (isNil(z = EVAL(car(x))))
      return Nil;
   Bind(y,f),  val(y) = z;
   x = prog(cdr(x));
   Unbind(f);
   return x;
}

// (use sym . prg) -> any
// (use (sym ..) . prg) -> any
any doUse(any x) {
   any y;

   x = cdr(x);
   if (isSym(y = car(x))) {
      bindFrame f;

      Bind(y,f);
      x = prog(cdr(x));
      Unbind(f);
   }
   else {
      struct {  // bindFrame
         struct bindFrame *link;
         int i, cnt;
         struct {any sym; any val;} bnd[length(y)];
      } f;

      f.link = Env.bind,  Env.bind = (bindFrame*)&f;
      f.i = f.cnt = 0;
      do {
         f.bnd[f.cnt].sym = car(y);
         f.bnd[f.cnt].val = val(car(y));
         ++f.cnt;
      } while (isCell(y = cdr(y)));
      x = prog(cdr(x));
      while (--f.cnt >= 0)
         val(f.bnd[f.cnt].sym) = f.bnd[f.cnt].val;
      Env.bind = f.link;
   }
   return x;
}

// (and 'any ..) -> any
any doAnd(any x) {
   x = cdr(x);
   do
      if (isNil(val(At) = EVAL(car(x))))
         return Nil;
   while (isCell(x = cdr(x)));
   return val(At);
}

// (or 'any ..) -> any
any doOr(any x) {
   x = cdr(x);
   do
      if (!isNil(val(At) = EVAL(car(x))))
         return val(At);
   while (isCell(x = cdr(x)));
   return Nil;
}

// (nand 'any ..) -> flg
any doNand(any x) {
   x = cdr(x);
   do
      if (isNil(val(At) = EVAL(car(x))))
         return T;
   while (isCell(x = cdr(x)));
   return Nil;
}

// (nor 'any ..) -> flg
any doNor(any x) {
   x = cdr(x);
   do
      if (!isNil(val(At) = EVAL(car(x))))
         return Nil;
   while (isCell(x = cdr(x)));
   return T;
}

// (xor 'any 'any) -> flg
any doXor(any x) {
   bool f;

   x = cdr(x),  f = isNil(EVAL(car(x))),  x = cdr(x);
   return  f ^ isNil(EVAL(car(x)))?  T : Nil;
}

// (bool 'any) -> flg
any doBool(any x) {return isNil(EVAL(cadr(x)))? Nil : T;}

// (not 'any) -> flg
any doNot(any x) {return isNil(EVAL(cadr(x)))? T : Nil;}

// (nil . prg) -> NIL
any doNil(any x) {
   while (isCell(x = cdr(x)))
      if (isCell(car(x)))
         evList(car(x));
   return Nil;
}

// (t . prg) -> T
any doT(any x) {
   while (isCell(x = cdr(x)))
      if (isCell(car(x)))
         evList(car(x));
   return T;
}

// (prog . prg) -> any
any doProg(any x) {return prog(cdr(x));}

// (prog1 'any1 . prg) -> any1
any doProg1(any x) {
   cell c1;

   x = cdr(x),  Push(c1, val(At) = EVAL(car(x)));
   while (isCell(x = cdr(x)))
      if (isCell(car(x)))
         evList(car(x));
   return Pop(c1);
}

// (prog2 'any1 'any2 . prg) -> any2
any doProg2(any x) {
   cell c1;

   x = cdr(x),  EVAL(car(x));
   x = cdr(x),  Push(c1, val(At) = EVAL(car(x)));
   while (isCell(x = cdr(x)))
      if (isCell(car(x)))
         evList(car(x));
   return Pop(c1);
}

// (if 'any1 'any2 . prg) -> any
any doIf(any x) {
   x = cdr(x);
   if (isNil(val(At) = EVAL(car(x))))
      return prog(cddr(x));
   x = cdr(x);
   return EVAL(car(x));
}

// (if2 'any1 'any2 'any3 'any4 'any5 . prg) -> any
any doIf2(any x) {
   x = cdr(x);
   if (isNil(val(At) = EVAL(car(x)))) {
      x = cdr(x);
      if (isNil(val(At) = EVAL(car(x))))
         return prog(cddddr(x));
      x = cdddr(x);
      return EVAL(car(x));
   }
   x = cdr(x);
   if (isNil(val(At) = EVAL(car(x)))) {
      x = cddr(x);
      return EVAL(car(x));
   }
   x = cdr(x);
   return EVAL(car(x));
}

// (ifn 'any1 'any2 . prg) -> any
any doIfn(any x) {
   x = cdr(x);
   if (!isNil(val(At) = EVAL(car(x))))
      return prog(cddr(x));
   x = cdr(x);
   return EVAL(car(x));
}

// (when 'any . prg) -> any
any doWhen(any x) {
   x = cdr(x);
   if (isNil(val(At) = EVAL(car(x))))
      return Nil;
   return prog(cdr(x));
}

// (unless 'any . prg) -> any
any doUnless(any x) {
   x = cdr(x);
   if (!isNil(val(At) = EVAL(car(x))))
      return Nil;
   return prog(cdr(x));
}

// (cond ('any1 . prg1) ('any2 . prg2) ..) -> any
any doCond(any x) {
   while (isCell(x = cdr(x)))
      if (!isNil(val(At) = EVAL(caar(x))))
         return prog(cdar(x));
   return Nil;
}

// (nond ('any1 . prg1) ('any2 . prg2) ..) -> any
any doNond(any x) {
   while (isCell(x = cdr(x)))
      if (isNil(val(At) = EVAL(caar(x))))
         return prog(cdar(x));
   return Nil;
}

// (case 'any (any1 . prg1) (any2 . prg2) ..) -> any
any doCase(any x) {
   any y, z;

   x = cdr(x),  val(At) = EVAL(car(x));
   while (isCell(x = cdr(x))) {
      y = car(x),  z = car(y);
      if (z == T  ||  equal(val(At), z))
         return prog(cdr(y));
      if (isCell(z)) {
         do
            if (equal(val(At), car(z)))
               return prog(cdr(y));
         while (isCell(z = cdr(z)));
      }
   }
   return Nil;
}

// (state 'var ((sym|lst sym [. prg]) . prg) ..) -> any
any doState(any ex) {
   any x, y, z;
   cell c1;

   x = cdr(ex);
   Push(c1, EVAL(car(x)));
   NeedVar(ex,data(c1));
   CheckVar(ex,data(c1));
   while (isCell(x = cdr(x))) {
      y = caar(x),  z = car(y);
      if (z==T || z==val(data(c1)) || isCell(z) && memq(val(data(c1)),z)) {
         y = cdr(y);
         if (!isCell(cdr(y)) || !isNil(val(At) = prog(cdr(y)))) {
            Touch(ex,data(c1));
            val(data(c1)) = car(y);
            drop(c1);
            return prog(cdar(x));
         }
      }
   }
   drop(c1);
   return Nil;
}

// (while 'any . prg) -> any
any doWhile(any x) {
   any cond;
   cell c1;

   cond = car(x = cdr(x)),  x = cdr(x);
   Push(c1, Nil);
   while (!isNil(val(At) = EVAL(cond)))
      data(c1) = prog(x);
   return Pop(c1);
}

// (until 'any . prg) -> any
any doUntil(any x) {
   any cond;
   cell c1;

   cond = car(x = cdr(x)),  x = cdr(x);
   Push(c1, Nil);
   while (isNil(val(At) = EVAL(cond)))
      data(c1) = prog(x);
   return Pop(c1);
}

// (loop ['any | (NIL 'any . prg) | (T 'any . prg) ..]) -> any
any doLoop(any ex) {
   any x, y;

   for (;;) {
      x = cdr(ex);
      do {
         if (isCell(y = car(x))) {
            if (isNil(car(y))) {
               y = cdr(y);
               if (isNil(val(At) = EVAL(car(y))))
                  return prog(cdr(y));
            }
            else if (car(y) == T) {
               y = cdr(y);
               if (!isNil(val(At) = EVAL(car(y))))
                  return prog(cdr(y));
            }
            else
               evList(y);
         }
      } while (isCell(x = cdr(x)));
   }
}

// (do 'flg|num ['any | (NIL 'any . prg) | (T 'any . prg) ..]) -> any
any doDo(any x) {
   any y, z;
   cell c1;

   x = cdr(x);
   if (isNil(data(c1) = EVAL(car(x))))
      return Nil;
   Save(c1);
   if (isNum(data(c1))) {
      if (isNeg(data(c1))) {
         drop(c1);
         return Nil;
      }
      data(c1) = bigCopy(data(c1));
   }
   x = cdr(x),  z = Nil;
   for (;;) {
      if (isNum(data(c1))) {
         if (IsZero(data(c1))) {
            drop(c1);
            return z;
         }
         digSub1(data(c1));
      }
      y = x;
      do {
         if (!isNum(z = car(y))) {
            if (isSym(z))
               z = val(z);
            else if (isNil(car(z))) {
               z = cdr(z);
               if (isNil(val(At) = EVAL(car(z)))) {
                  drop(c1);
                  return prog(cdr(z));
               }
               z = Nil;
            }
            else if (car(z) == T) {
               z = cdr(z);
               if (!isNil(val(At) = EVAL(car(z)))) {
                  drop(c1);
                  return prog(cdr(z));
               }
               z = Nil;
            }
            else
               z = evList(z);
         }
      } while (isCell(y = cdr(y)));
   }
}

// (at '(cnt1 . cnt2) . prg) -> any
any doAt(any ex) {
   any x;

   x = cdr(ex),  x = EVAL(car(x));
   NeedCell(ex,x);
   NeedCnt(ex,car(x));
   NeedCnt(ex,cdr(x));
   if (num(setDig(car(x), unDig(car(x))+2)) < unDig(cdr(x)))
      return Nil;
   setDig(car(x), 0);
   return prog(cddr(ex));
}

// (for sym|(sym2 . sym) 'lst ['any | (NIL 'any . prg) | (T 'any . prg) ..]) -> any
// (for (sym|(sym2 . sym) 'any1 'any2 [. prg]) ['any | (NIL 'any . prg) | (T 'any . prg) ..]) -> any
any doFor(any ex) {
   any x, y, body, cond;
   cell c1;
   struct {  // bindFrame
      struct bindFrame *link;
      int i, cnt;
      struct {any sym; any val;} bnd[2];
   } f;

   f.link = Env.bind,  Env.bind = (bindFrame*)&f;
   f.i = 0;
   if (!isCell(y = car(x = cdr(ex))) || !isCell(cdr(y))) {
      if (!isCell(y)) {
         f.cnt = 1;
         f.bnd[0].sym = y;
         f.bnd[0].val = val(y);
      }
      else {
         f.cnt = 2;
         f.bnd[0].sym = cdr(y);
         f.bnd[0].val = val(cdr(y));
         f.bnd[1].sym = car(y);
         f.bnd[1].val = val(car(y));
         val(f.bnd[1].sym) = Zero;
      }
      y = Nil;
      x = cdr(x),  Push(c1, EVAL(car(x)));
      body = x = cdr(x);
      while (isCell(data(c1))) {
         val(f.bnd[0].sym) = car(data(c1)),  data(c1) = cdr(data(c1));
         if (f.cnt == 2) {
            val(f.bnd[1].sym) = bigCopy(val(f.bnd[1].sym));
            digAdd(val(f.bnd[1].sym), 2);
         }
         do {
            if (!isNum(y = car(x))) {
               if (isSym(y))
                  y = val(y);
               else if (isNil(car(y))) {
                  y = cdr(y);
                  if (isNil(val(At) = EVAL(car(y)))) {
                     y = prog(cdr(y));
                     goto for1;
                  }
                  y = Nil;
               }
               else if (car(y) == T) {
                  y = cdr(y);
                  if (!isNil(val(At) = EVAL(car(y)))) {
                     y = prog(cdr(y));
                     goto for1;
                  }
                  y = Nil;
               }
               else
                  y = evList(y);
            }
         } while (isCell(x = cdr(x)));
         x = body;
      }
   for1:
      drop(c1);
      if (f.cnt == 2)
         val(f.bnd[1].sym) = f.bnd[1].val;
      val(f.bnd[0].sym) = f.bnd[0].val;
      Env.bind = f.link;
      return y;
   }
   if (!isCell(car(y))) {
      f.cnt = 1;
      f.bnd[0].sym = car(y);
      f.bnd[0].val = val(car(y));
   }
   else {
      f.cnt = 2;
      f.bnd[0].sym = cdar(y);
      f.bnd[0].val = val(cdar(y));
      f.bnd[1].sym = caar(y);
      f.bnd[1].val = val(caar(y));
      val(f.bnd[1].sym) = Zero;
   }
   y = cdr(y);
   val(f.bnd[0].sym) = EVAL(car(y));
   y = cdr(y),  cond = car(y),  y = cdr(y);
   Push(c1,Nil);
   body = x = cdr(x);
   while (!isNil(val(At) = EVAL(cond))) {
      if (f.cnt == 2) {
         val(f.bnd[1].sym) = bigCopy(val(f.bnd[1].sym));
         digAdd(val(f.bnd[1].sym), 2);
      }
      do {
         if (!isNum(data(c1) = car(x))) {
            if (isSym(data(c1)))
               data(c1) = val(data(c1));
            else if (isNil(car(data(c1)))) {
               data(c1) = cdr(data(c1));
               if (isNil(val(At) = EVAL(car(data(c1))))) {
                  data(c1) = prog(cdr(data(c1)));
                  goto for2;
               }
               data(c1) = Nil;
            }
            else if (car(data(c1)) == T) {
               data(c1) = cdr(data(c1));
               if (!isNil(val(At) = EVAL(car(data(c1))))) {
                  data(c1) = prog(cdr(data(c1)));
                  goto for2;
               }
               data(c1) = Nil;
            }
            else
               data(c1) = evList(data(c1));
         }
      } while (isCell(x = cdr(x)));
      if (isCell(y))
         val(f.bnd[0].sym) = prog(y);
      x = body;
   }
for2:
   if (f.cnt == 2)
      val(f.bnd[1].sym) = f.bnd[1].val;
   val(f.bnd[0].sym) = f.bnd[0].val;
   Env.bind = f.link;
   return Pop(c1);
}

static any Thrown;

// (catch 'sym . prg) -> any
any doCatch(any ex) {
   any x, y;
   catchFrame f;

   x = cdr(ex),  f.tag = EVAL(car(x));
   NeedSym(ex,f.tag);
   f.link = CatchPtr,  CatchPtr = &f;
   f.env = Env;
   y = setjmp(f.rst)? Thrown : prog(cdr(x));
   CatchPtr = f.link;
   return y;
}

// (throw 'sym 'any)
any doThrow(any ex) {
   any x, tag;
   catchFrame *p;

   x = cdr(ex),  tag = EVAL(car(x));
   x = cdr(x),  Thrown = EVAL(car(x));
   for (p = CatchPtr;  p;  p = p->link)
      if (p->tag == T  ||  tag == p->tag) {
         unwind(p);
         longjmp(p->rst, 1);
      }
   err(ex, tag, "Tag not found");
}

// (finally exe . prg) -> any
any doFinally(any x) {
   catchFrame f;
   cell c1;

   x = cdr(x);
   f.tag = car(x);
   f.link = CatchPtr,  CatchPtr = &f;
   f.env = Env;
   Push(c1, prog(cdr(x)));
   EVAL(f.tag);
   CatchPtr = f.link;
   return Pop(c1);
}

static outFrame Out;
static struct {  // bindFrame
   struct bindFrame *link;
   int i, cnt;
   struct {any sym; any val;} bnd[3];  // for 'Up', 'Run' and 'At'
} Brk;

void brkLoad(any x) {
   if (!isNil(val(Dbg)) && !Env.brk) {
      if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
         err(x, NULL, "BREAK");
      Env.brk = YES;
      Brk.cnt = 3;
      Brk.bnd[0].sym = Up,  Brk.bnd[0].val = val(Up),  val(Up) = x;
      Brk.bnd[1].sym = Run,  Brk.bnd[1].val = val(Run),  val(Run) = Nil;
      Brk.bnd[2].sym = At,  Brk.bnd[2].val = val(At);
      Brk.link = Env.bind,  Env.bind = (bindFrame*)&Brk;
      Out.pid = -1,  Out.fd = 1,  pushOutFiles(&Out);
      print(x), crlf();
      load(NULL, '!', Nil);
      popOutFiles();
      val(At) = Brk.bnd[2].val;
      val(Run) = Brk.bnd[1].val;
      val(Up) = Brk.bnd[0].val;
      Env.bind = Brk.link;
      Env.brk = NO;
   }
}

// (! . prg) -> any
any doBreak(any ex) {
   brkLoad(cdr(ex));
   return EVAL(cdr(ex));
}

// (e . prg) -> any
any doE(any ex) {
   any x;
   cell c1, at, key;

   if (!Env.brk)
      err(ex, NULL, "No Break");
   Push(c1,val(Dbg)),  val(Dbg) = Nil;
   Push(at, val(At)),  val(At) = Brk.bnd[2].val;
   Push(key, val(Run)),  val(Run) = Brk.bnd[1].val;
   if (Env.inFiles) {
      Env.get = Env.inFiles->get;
      if (!Env.inFiles->link  || Env.inFiles->link->pid < 0)
         InFile = NULL,  Chr = Next0;
      else if (InFile = InFiles[Env.inFiles->link->fd])
         Chr = InFile->next;
      else
         Chr = Next0;
   }
   popOutFiles();
   x = isCell(cdr(ex))? prog(cdr(ex)) : EVAL(val(Up));
   pushOutFiles(&Out);
   if (InFile)
      InFile->next = Chr;
   else
      Next0 = Chr;
   InFile = NULL,  OutFile = NULL,  Chr = 0;
   val(Run) = data(key);
   val(At) = data(at);
   val(Dbg) = Pop(c1);
   return x;
}

static void traceIndent(int i, any x, char *s) {
   if (i > 64)
      i = 64;
   while (--i >= 0)
      Env.put(' ');
   if (isSym(x))
      print(x);
   else
      print(car(x)), space(), print(cdr(x)), space(), print(val(This));
   outString(s);
}

static void traceSym(any x) {
   if (x != At)
      space(), print(val(x));
   else {
      int i = Env.next;

      while (--i >= 0)
         space(), print(data(Env.arg[i]));
   }
}

// ($ sym|lst lst . prg) -> any
any doTrace(any x) {
   any foo, body;
   outFile *oSave = OutFile;
   FILE *stdSave = StdOut;
   void (*putSave)(int) = Env.put;
   cell c1;

   if (isNil(val(Dbg)))
      return prog(cdddr(x));
   OutFile = NULL,  StdOut = stderr,  Env.put = putStdout;
   x = cdr(x),  foo = car(x);
   x = cdr(x),  body = cdr(x);
   traceIndent(++Env.trace, foo, " :");
   for (x = car(x);  isCell(x);  x = cdr(x))
      traceSym(car(x));
   if (!isNil(x) && isSym(x))
      traceSym(x);
   crlf();
   Env.put = putSave,  OutFile = oSave,  StdOut = stdSave;
   Push(c1, prog(body));
   OutFile = NULL,  StdOut = stderr;
   Env.put = putStdout;
   traceIndent(Env.trace--, foo, " = "),  print(data(c1)),  crlf();
   Env.put = putSave,  OutFile = oSave,  StdOut = stdSave;
   return Pop(c1);
}

// (sys 'any ['any]) -> sym
any doSys(any x) {
   any y;

   y = evSym(x = cdr(x));
   {
      char nm[bufSize(y)];

      bufString(y,nm);
      if (!isCell(x = cdr(x)))
         return mkStr(getenv(nm));
      y = evSym(x);
      {
         char val[bufSize(y)];

         bufString(y,val);
         return setenv(nm,val,1)? Nil : y;
      }
   }
}

// (call 'any ..) -> flg
any doCall(any ex) {
   pid_t pid;
   any x, y;
   int res, i, ac = length(x = cdr(ex));
   char *av[ac+1];

   if (ac == 0)
      return Nil;
   i = 0;  do {
      y = evSym(x),  x = cdr(x);
      av[i] = alloc(NULL, bufSize(y)),  bufString(y, av[i]);
   } while (++i < ac);
   av[ac] = NULL;
   if ((pid = fork()) == 0) {
      setpgid(0,0);
      execvp(av[0], (char**)av);
      execError(av[0]);
   }
   i = 0;  do
      free(av[i]);
   while (++i < ac);
   if (pid < 0)
      err(ex, NULL, "fork");
   setpgid(pid,0);
   if (Termio)
      tcsetpgrp(0,pid);
   for (;;) {
      while (waitpid(pid, &res, WUNTRACED) < 0) {
         if (errno != EINTR)
            err(ex, NULL, "wait pid");
         if (Signal)
            sighandler(ex);
      }
      if (Termio)
         tcsetpgrp(0,getpgrp());
      if (!WIFSTOPPED(res))
         return res == 0? T : Nil;
      load(NULL, '+', Nil);
      if (Termio)
         tcsetpgrp(0,pid);
      kill(pid, SIGCONT);
   }
}

// (tick (cnt1 . cnt2) . prg) -> any
any doTick(any ex) {
   any x;
   clock_t n1, n2, save1, save2;
   struct tms tim;
   static clock_t ticks1, ticks2;

   save1 = ticks1,  save2 = ticks2;
   times(&tim),  n1 = tim.tms_utime,  n2 = tim.tms_stime;
   x = prog(cddr(ex));
   times(&tim);
   n1 = (tim.tms_utime - n1) - (ticks1 - save1);
   n2 = (tim.tms_stime - n2) - (ticks2 - save2);
   setDig(caadr(ex), unDig(caadr(ex)) + 2*n1);
   setDig(cdadr(ex), unDig(cdadr(ex)) + 2*n2);
   ticks1 += n1,  ticks2 += n2;
   return x;
}

// (ipid) -> pid | NIL
any doIpid(any ex __attribute__((unused))) {
   if (Env.inFiles  &&  Env.inFiles->pid > 0)
      return boxCnt((long)Env.inFiles->pid);
   return Nil;
}

// (opid) -> pid | NIL
any doOpid(any ex __attribute__((unused))) {
   if (Env.outFiles  &&  Env.outFiles->pid > 0)
      return boxCnt((long)Env.outFiles->pid);
   return Nil;
}

// (kill 'pid ['cnt]) -> flg
any doKill(any ex) {
   pid_t pid;

   pid = (pid_t)evCnt(ex,cdr(ex));
   return kill(pid, isCell(cddr(ex))? (int)evCnt(ex,cddr(ex)) : SIGTERM)? Nil : T;
}

static int allocChild(void) {
   int i;

   for (i = 0; i < Children; ++i)
      if (!Child[i].pid)
         return i;
   return i;
}

static void allocChildren(void) {
   int i;

   Child = alloc(Child, (Children + 8) * sizeof(child));
   for (i = 0; i < 8; ++i)
      Child[Children++].pid = 0;
}

pid_t forkLisp(any ex) {
   pid_t n;
   inFrame *in;
   outFrame *out;
   int i, hear[2], tell[2];
   static int mic[2];

   fflush(NULL);
   if (!Spkr) {
      if (pipe(mic) < 0)
         pipeError(ex, "open");
      Spkr = mic[0];
   }
   if (pipe(hear) < 0  ||  pipe(tell) < 0)
      pipeError(ex, "open");
   i = allocChild();
   if ((n = fork()) < 0)
      err(ex, NULL, "fork");
   if (n == 0) {
      /* Child Process */
      for (in = Env.inFiles; in; in = in->link)
         if (in->pid > 0)
            in->pid = 0;
      for (out = Env.outFiles; out; out = out->link)
         if (out->pid > 0)
            out->pid = 0;
      free(Termio),  Termio = NULL;
      if (close(hear[1]) < 0  ||  close(tell[0]) < 0  ||  close(mic[0]) < 0)
         pipeError(ex, "close");
      Slot = i;
      Spkr = 0;
      Mic = mic[1];
      for (i = 0; i < Children; ++i)
         if (Child[i].pid)
            close(Child[i].hear), close(Child[i].tell),  free(Child[i].buf);
      Children = 0,  free(Child),  Child = NULL;
      if (Hear)
         close(Hear),  closeInFile(Hear);
      initInFile(Hear = hear[0], NULL);
      if (Tell)
         close(Tell);
      Tell = tell[1];
      val(PPid) = val(Pid);
      val(Pid) = boxCnt(getpid());
      run(val(Fork));
      return 0;
   }
   if (i == Children)
      allocChildren();
   if (close(hear[0]) < 0  ||  close(tell[1]) < 0)
      pipeError(ex, "close");
   Child[i].pid = n;
   Child[i].hear = tell[0];
   blocking(NO, ex, Child[i].tell = hear[1]);
   Child[i].ofs = Child[i].cnt = 0;
   Child[i].buf = NULL;
   return n;
}

// (fork) -> pid | NIL
any doFork(any ex) {
   int n;

   return (n = forkLisp(ex))? boxCnt(n) : Nil;
}

// (bye 'cnt|NIL)
any doBye(any ex) {
   any x = EVAL(cadr(ex));

   bye(isNil(x)? 0 : xCnt(ex,x));
}
