
#include "clang_visitor.hpp"

#define DEBUG printf("DEBUG :: >>> %s %d ... \n", __PRETTY_FUNCTION__, __LINE__)

static shared_ptr<Node> toNode(const ASTContext *ctx, const PresumedLoc &loc,
                               const APInt &ii) {
  if (ii.getBitWidth() <= 64) {
    return shared_ptr<IntegerNode>(
        new IntegerNode(loc.getLine(), loc.getColumn(), ii.getSExtValue()));
  } else {
    return shared_ptr<SymbolNode>(new SymbolNode(
        loc.getLine(), loc.getColumn(), ii.toString(10, ii.isSignBit())));
  }
}

static shared_ptr<SymbolNode> toNode(const ASTContext *ctx,
                                     const PresumedLoc &loc, const Expr *e) {
  LangOptions LO;
  std::string str;
  raw_string_ostream ros(str);
  e->printPretty(ros, nullptr, ctx->getLangOpts());
  return shared_ptr<SymbolNode>(
      new SymbolNode(loc.getLine(), loc.getColumn(), str));
}

static vector<shared_ptr<Node> >
toNode(const ASTContext *ctx, const PresumedLoc &loc, const Qualifiers &quals) {
  vector<shared_ptr<Node> > res;

  if (quals.hasConst()) {
    res.push_back(shared_ptr<SymbolNode>(
        new SymbolNode(loc.getLine(), loc.getColumn(), "const")));
  }
  if (quals.hasVolatile()) {
    res.push_back(shared_ptr<SymbolNode>(
        new SymbolNode(loc.getLine(), loc.getColumn(), "volatile")));
  }
  if (quals.hasRestrict()) {
    res.push_back(shared_ptr<SymbolNode>(
        new SymbolNode(loc.getLine(), loc.getColumn(), "restrict")));
  }
  if (quals.hasAddressSpace()) {
    unsigned addressSpace = quals.getAddressSpace();
    switch (addressSpace) {
    case LangAS::opencl_global:
      res.push_back(shared_ptr<SymbolNode>(
          new SymbolNode(loc.getLine(), loc.getColumn(), "__global")));
      break;
    case LangAS::opencl_local:
      res.push_back(shared_ptr<SymbolNode>(
          new SymbolNode(loc.getLine(), loc.getColumn(), "__local")));
      break;
    case LangAS::opencl_constant:
      res.push_back(shared_ptr<SymbolNode>(
          new SymbolNode(loc.getLine(), loc.getColumn(), "__constant")));
      break;
    case LangAS::cuda_constant:
      DEBUG;
      res.push_back(shared_ptr<SymbolNode>(
          new SymbolNode(loc.getLine(), loc.getColumn(), "__constant__")));
      break;
    case LangAS::cuda_device:
      DEBUG;
      res.push_back(shared_ptr<SymbolNode>(
          new SymbolNode(loc.getLine(), loc.getColumn(), "__device__")));
      break;
    case LangAS::cuda_shared:
      DEBUG;
      res.push_back(shared_ptr<SymbolNode>(
          new SymbolNode(loc.getLine(), loc.getColumn(), "__shared__")));
      break;
    default: {
      DEBUG;
      ostringstream o;
      o << "__attribute__((address_space(";
      o << addressSpace;
      o << ")))";
      res.push_back(shared_ptr<SymbolNode>(
          new SymbolNode(loc.getLine(), loc.getColumn(), o.str())));
    }
    }
  }
  return res;
}

static shared_ptr<TypeNode> toNode(const ASTContext *ctx,
                                   const PresumedLoc &loc, const QualType &typ);
static shared_ptr<TypeNode> toNode(const ASTContext *ctx,
                                   const PresumedLoc &loc, const Type *ty) {
  if (const BuiltinType *bty = dyn_cast<const BuiltinType>(ty)) {
    StringRef s = bty->getName(PrintingPolicy(ctx->getLangOpts()));
    return shared_ptr<TypeNode>(
        new TypeNode(loc.getLine(), loc.getColumn(), s.str()));
  } else if (ty->isPointerType()) {
    auto t = toNode(ctx, loc, ty->getPointeeType());
    return shared_ptr<TypeNode>(
        new ReferenceTypeNode(loc.getLine(), loc.getColumn(), t));
  } else {
    auto typ = QualType::getAsString(QualType(ty, 0).getSplitDesugaredType());
    return shared_ptr<TypeNode>(
        new TypeNode(loc.getLine(), loc.getColumn(), typ));
  }
}

static shared_ptr<TypeNode>
toNode(const ASTContext *ctx, const PresumedLoc &loc, const QualType &typ) {
  shared_ptr<TypeNode> res;
  res = toNode(ctx, loc, typ.getTypePtr());
  if (typ.hasQualifiers()) {
    auto quals = toNode(ctx, loc, typ.getQualifiers());
    for (auto q : quals) {
      res->addQualifyer(q);
    }
  }
  return res;
}
static shared_ptr<IdentifierNode>
toNode(const ASTContext *ctx, const PresumedLoc &loc, const Decl *decl) {
  const NamedDecl *ND = dyn_cast<NamedDecl>(decl);
  shared_ptr<IdentifierNode> nd(
      new IdentifierNode(loc.getLine(), loc.getColumn()));
  if (ND) {
    nd->setName(ND->getNameAsString());
    nd->isHidden(ND->isHidden());
  }
  if (const ValueDecl *VD = dyn_cast<ValueDecl>(decl)) {
    nd->setType(toNode(ctx, loc, VD->getType()));
  }
  return nd;
}

#if 0

  SymbolicExpr toSymbolicExpr(const Expr * e) {
      SymbolicExpr expr = SymbolicExpr(this);
      expr <<= toSymbolicLiteral(e);
      return expr;
  }

  SymbolicExpr toSymbolicExpr(const Type * ty) {
      if (const ConstantArrayType* arryT =
          dyn_cast<const ConstantArrayType>(ty)) {
          SymbolicLiteral len(toSymbolicLiteral(arryT->getSize()));
          SymbolicTypeExpr typ(this);
          typ <<= toSymbolicExpr(arryT->getElementType());
          SymbolicArrayExpr arry(this);
          arry <<= typ;
          arry <<= len;
          return arry;
      } else if (const VariableArrayType* arryT =
                 dyn_cast<const VariableArrayType>(ty)) {
          SymbolicExpr typ = toSymbolicExpr(arryT->getElementType());
          SymbolicExpr arry = SymbolicArrayExpr(this);
          arry <<= typ;
          if (arryT->getIndexTypeQualifiers().hasQualifiers()) {
              arry <<= toSymbolicExpr(arryT->getIndexTypeQualifiers());
          }
          if (arryT->getSizeModifier() == VariableArrayType::Static) {
              arry <<= "static";
          } else if (arryT->getSizeModifier() == VariableArrayType::Star) {
              arry <<= "*";
          }

          arry <<= SymbolicArraySizeExpr(this, toSymbolicLiteral(arryT->getSizeExpr()));
          return arry;
      }
      /*
       } else if (const IncompleteArrayType* arryT =
       dyn_cast<const IncompleteArrayType>(ty)) {
       SymbolicExpr arry = CArray(this);
       SymbolicExpr typ = toSymbolicExpr(arryT->getElementType());
       arry << qualExp;
       arry << typ;
       return arry;
       } else if (const PointerType* pty = dyn_cast<const PointerType>(ty)) {
       const QualType pointee_ty = pty->getPointeeType();
       std::string name_ptr =  "* " + name;
       result = printDecl(pointee_ty, name_ptr, array_dim);
       } else if (const ElaboratedType* ety =
       dyn_cast<const ElaboratedType>(ty)) {
       if (const RecordType* rty =
       dyn_cast<const RecordType>(ety->getNamedType().getTypePtr())) {
       TagDecl* tdecl = rty->getDecl();
       std::string def =
       rty->getDecl()->getName().str() + std::string(" ") + name;
       if (tdecl->isStruct()) result = "struct " + def;
       else if (tdecl->isUnion())  result = "union " + def;
       else {
       assert(false &&
       "PromoteDecl: only struct/union are allowed as record type.");
       }
       } else {
       assert(false && "PromoteDecl: unsupported elaborated type.");
       }
       } else if (const BuiltinType* bty = dyn_cast<const BuiltinType>(ty)) {
       LangOptions LO;
       result = bty->getName(PrintingPolicy(LO)) + std::string(" ") + name;
       } else if (const TypedefType* tty = dyn_cast<const TypedefType>(ty)) {
       result = tty->getDecl()->getName().str() + std::string(" ") + name;
       } else if (const ParenType* pty = dyn_cast<const ParenType>(ty)) {
       QualType inner_ty = pty->getInnerType();
       std::string name_ptr =  "(" + name;
       result = printDecl(inner_ty, name_ptr, array_dim);
       result = result + ")";
       } else {
       llvm::errs() << "Error: " << ty->getTypeClassName() << '\n';
       assert(false && "PromoteDecl: unsupported type is found.");
       }
       */
      else if (const BuiltinType* bty = dyn_cast<const BuiltinType>(ty)) {
          SymbolicLiteral lit = bty->getName(PrintingPolicy(ctx->getLangOpts()));
          SymbolicTypeExpr exp(this);
          exp <<= lit;
          return exp;
      }
      return SymbolicNoneExpr(this);
  }

#endif
/*******************************************************************************************************/
/*******************************************************************************************************/
/*******************************************************************************************************/
/*******************************************************************************************************/
/*******************************************************************************************************/

SVisitor::SVisitor(CompilerInstance &CI)
    : ctx(&(CI.getASTContext())), Traits(ctx->getCommentCommandTraits()),
      SM(ctx->getSourceManager()) {
  prog_ = shared_ptr<ProgramNode>(new ProgramNode());
  current_node = prog_;
}

shared_ptr<ProgramNode> SVisitor::getProgram() { return prog_; }
bool SVisitor::canIgnoreCurrentASTNode() const {
  const DeclContext *decl = ctx->getTranslationUnitDecl();
  for (auto it = decl->decls_begin(), declEnd = decl->decls_end();
       it != declEnd; ++it) {
    auto startLocation = (*it)->getLocStart();
    if (startLocation.isValid() &&
        SM.getMainFileID() == SM.getFileID(startLocation)) {
      return false;
    }
  }
  //DEBUG;
  return true;
}
bool SVisitor::TraverseFunctionDecl(FunctionDecl *decl) {
  //  if (canIgnoreCurrentASTNode()) {
  //   return true;
  //}
  PresumedLoc loc = SM.getPresumedLoc(decl->getPointOfInstantiation());
  shared_ptr<Node> tmp = current_node;
  shared_ptr<FunctionNode> func(
      new FunctionNode(loc.getLine(), loc.getColumn()));
  shared_ptr<TypeNode> returnType = toNode(ctx, loc, decl->getReturnType());
  shared_ptr<IdentifierNode> name(
      new IdentifierNode(loc.getLine(), loc.getColumn(),
                         decl->getNameInfo().getName().getAsString()));

  func->setReturnType(returnType);
  func->setName(name);
  current_node = tmp;
  unsigned paramCount = decl->getNumParams();
  std::cout << "paramCount = " << paramCount << std::endl;
  for (unsigned i = 0; i < paramCount; i++) {
    TraverseDecl(decl->getParamDecl(i));
    func->addParameter(current_node);
  }
  if (decl->doesThisDeclarationHaveABody()) {
    shared_ptr<BlockNode> body = func->getBody();
    current_node = body;
    //decl->getBody()->dumpColor();
    SVisitor::TraverseStmt(decl->getBody());
    *body <<= current_node;
  }
  if (decl->hasAttr<CUDAGlobalAttr>()) {
    decl->dumpColor();
    func->addAttribute("__global__");
  }
  if (decl->hasAttr<CUDADeviceAttr>()) {
    func->addAttribute("__device__");
  }
  if (decl->hasAttr<CUDAHostAttr>()) {
    func->addAttribute("__host__");
  }
  current_node = func;
  return true;
}
bool SVisitor::TraverseParmVarDecl(ParmVarDecl *decl) {
  PresumedLoc loc = SM.getPresumedLoc(decl->getSourceRange().getBegin());
  shared_ptr<Node> tmp = current_node;
  shared_ptr<ParameterNode> nd(
      new ParameterNode(loc.getLine(), loc.getColumn()));
  shared_ptr<TypeNode> typ = toNode(ctx, loc, decl->getType());
  shared_ptr<IdentifierNode> id(new IdentifierNode(
      loc.getLine(), loc.getColumn(), decl->getNameAsString()));

  nd->setType(typ);
  nd->setIdentifier(id);

  current_node = nd;
  return true;
}

bool SVisitor::TraverseVarDecl(VarDecl *decl) {
  PresumedLoc loc = SM.getPresumedLoc(decl->getSourceRange().getBegin());
  shared_ptr<Node> tmp = current_node;
  shared_ptr<DeclareNode> nd(new DeclareNode(loc.getLine(), loc.getColumn()));
  shared_ptr<TypeNode> typ = toNode(ctx, loc, decl->getType());
  shared_ptr<IdentifierNode> id(new IdentifierNode(
      loc.getLine(), loc.getColumn(), decl->getNameAsString()));

  nd->setType(typ);
  nd->setIdentifier(id);

  if (decl->hasInit()) {
    current_node = nd;
    SVisitor::TraverseStmt(decl->getInit());
    // decl->getInit()->dump();
    assert(current_node != nd);
    nd->setInitializer(current_node);
  }
  current_node = nd;
  return true;
}

bool SVisitor::TraverseDeclStmt(DeclStmt *decl) {
  PresumedLoc loc = SM.getPresumedLoc(decl->getSourceRange().getBegin());
  shared_ptr<Node> tmp = current_node;
  shared_ptr<CompoundNode> nd(new CompoundNode(loc.getLine(), loc.getColumn()));
  for (auto init = decl->decl_begin(), end = decl->decl_end(); init != end;
       ++init) {
    current_node = tmp;
    SVisitor::TraverseDecl(*init);
    assert(current_node != tmp);
    *nd <<= current_node;
  }
  current_node = nd;
  return true;
}

bool SVisitor::TraverseWhileStmt(WhileStmt *stmt) {
  if (canIgnoreCurrentASTNode()) {
	  //DEBUG;
	  //stmt->dumpColor();
    return true;
  }
  PresumedLoc loc = SM.getPresumedLoc(stmt->getWhileLoc());
  shared_ptr<WhileNode> nd(new WhileNode(loc.getLine(), loc.getColumn()));

  current_node = nd;
  SVisitor::TraverseStmt(stmt->getCond());

  current_node =
      shared_ptr<BlockNode>(new BlockNode(loc.getLine(), loc.getColumn()));
  SVisitor::TraverseStmt(stmt->getBody());
  *nd <<= current_node;

  current_node = nd;
  return true;
}

bool SVisitor::TraverseForStmt(ForStmt *stmt) {
  PresumedLoc loc = SM.getPresumedLoc(stmt->getLocStart());
  shared_ptr<ForNode> nd(new ForNode(loc.getLine(), loc.getColumn()));
  if (stmt->getInit()) {
    current_node = nd;
    SVisitor::TraverseStmt(stmt->getInit());
    nd->setInit(current_node);
  }
  if (stmt->getCond()) {
    current_node = nd;
    SVisitor::TraverseStmt(stmt->getCond());
    nd->setCond(current_node);
  }
  if (stmt->getInc()) {
    current_node = nd;
    SVisitor::TraverseStmt(stmt->getInc());
    nd->setInc(current_node);
  }
  if (stmt->getBody()) {
    current_node = nd;
    SVisitor::TraverseStmt(stmt->getBody());
    nd->setBody(current_node);
  }
  current_node = nd;
  return true;
}
bool SVisitor::TraverseIfStmt(IfStmt *stmt) {
  shared_ptr<Node> tmp = current_node;
  PresumedLoc loc = SM.getPresumedLoc(stmt->getIfLoc());
  shared_ptr<IfNode> nd(new IfNode(loc.getLine(), loc.getColumn()));

  current_node = nd;
  SVisitor::TraverseStmt(stmt->getCond());
  nd->setCondition(current_node);

  current_node = nd;
  SVisitor::TraverseStmt(stmt->getThen());
  if (!current_node->isBlock()) {
    shared_ptr<BlockNode> blk(new BlockNode(loc.getLine(), loc.getColumn()));
    *blk <<= current_node;
    current_node = blk;
  }
  nd->setThen(current_node);

  if (stmt->getElse() != nullptr) {
    current_node = nd;
    SVisitor::TraverseStmt(stmt->getElse());
    if (!current_node->isBlock()) {
      shared_ptr<BlockNode> blk(new BlockNode(loc.getLine(), loc.getColumn()));
      *blk <<= current_node;
      current_node = blk;
    }
    nd->setElse(current_node);
  }
  current_node = nd;

  return true;
}

bool SVisitor::TraverseConditionalOperator(ConditionalOperator * E) {
  
  PresumedLoc loc = SM.getPresumedLoc(E->getExprLoc());
  shared_ptr<ConditionalNode> nd(new ConditionalNode(loc.getLine(), loc.getColumn()));
  current_node = nd;
  TraverseStmt(E->getCond());
  nd->setCondition(current_node);
  current_node = nd;
  TraverseStmt(E->getLHS());
  nd->setThen(current_node);
  current_node = nd;
  TraverseStmt(E->getRHS());
  nd->setElse(current_node);
  current_node = nd;
  return true;
}


bool SVisitor::TraverseArraySubscriptExpr(ArraySubscriptExpr * E) {
  
  PresumedLoc loc = SM.getPresumedLoc(E->getExprLoc());
  shared_ptr<SubscriptNode> nd(new SubscriptNode(loc.getLine(), loc.getColumn()));
  current_node = nd;
  TraverseStmt(E->getLHS());
  nd->setLHS(current_node);
  current_node = nd;
  TraverseStmt(E->getRHS());
  nd->setRHS(current_node);
  current_node = nd;
  return true;
}
bool SVisitor::TraverseCompoundStmt(CompoundStmt *stmt) {
  shared_ptr<Node> tmp = current_node;
  PresumedLoc loc = SM.getPresumedLoc(stmt->getLocStart());
  shared_ptr<CompoundNode> nd(new CompoundNode(loc.getLine(), loc.getColumn()));

  for (auto init = stmt->body_begin(), end = stmt->body_end(); init != end;
       ++init) {
    current_node = nd;
    SVisitor::TraverseStmt(*init);
    *nd <<= current_node;
  }
  current_node = nd;
  return true;
}

bool SVisitor::TraverseReturnStmt(ReturnStmt *stmt) {
  PresumedLoc loc = SM.getPresumedLoc(stmt->getReturnLoc());
  shared_ptr<ReturnNode> nd(new ReturnNode(loc.getLine(), loc.getColumn()));

  current_node = nd;
  if (stmt->getRetValue()) {
    SVisitor::TraverseStmt(stmt->getRetValue());

    nd->setReturnValue(current_node);
  }

  current_node = nd;
  return true;
}
bool SVisitor::handleUnaryOperator(UnaryOperator *E) {
  PresumedLoc loc = SM.getPresumedLoc(E->getExprLoc());
  shared_ptr<UnaryOperatorNode> nd(
      new UnaryOperatorNode(loc.getLine(), loc.getColumn()));
  current_node = nd;
  nd->setOperator(E->getOpcodeStr(E->getOpcode()));
  SVisitor::TraverseStmt(E->getSubExpr());
  nd->setArg(current_node);
  current_node = nd;
  return true;
}

#define OPERATOR(NAME)                                                         \
  bool SVisitor::TraverseUnary##NAME(UnaryOperator *E) {                       \
    return handleUnaryOperator(E);                                             \
  }

UNARYOP_LIST()
#undef OPERATOR
bool SVisitor::TraverseUnaryOperator(UnaryOperator *op) {
  return handleUnaryOperator(op);
}
bool SVisitor::TraverseUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr *E) {
  string fun;
  switch (E->getKind()) {
    case UETT_SizeOf:
    fun = "sizeof";
    break ;
    case UETT_AlignOf:
    fun = "alignof";
    break ;
    case UETT_VecStep  :
    fun = "vecstep";
    break ;
  }
  PresumedLoc loc = SM.getPresumedLoc(E->getExprLoc());
  shared_ptr<CallNode> nd(
      new CallNode(loc.getLine(), loc.getColumn()));
  nd->setFunction(shared_ptr<IdentifierNode>(new IdentifierNode(loc.getLine(), loc.getColumn(), fun)));
  current_node = nd;
  if (E->isArgumentType()) {
    current_node = toNode(ctx, loc, E->getArgumentType());
} else {

    SVisitor::TraverseStmt(E->getArgumentExpr());
}
  nd->addArg(current_node);
  current_node = nd;
  return true;
}

bool SVisitor::handleBinaryOperator(BinaryOperator *E) {
  PresumedLoc loc = SM.getPresumedLoc(E->getOperatorLoc());
  if (E->isAssignmentOp()) {
    shared_ptr<AssignNode> nd(new AssignNode(loc.getLine(), loc.getColumn()));
    current_node = nd;
    SVisitor::TraverseStmt(E->getLHS());
    assert(current_node != nd);
    nd->setLHS(current_node);
    current_node = nd;
    SVisitor::TraverseStmt(E->getRHS());
    assert(current_node != nd);
    nd->setRHS(current_node);
    current_node = nd;
  } else {
    shared_ptr<BinaryOperatorNode> nd(
        new BinaryOperatorNode(loc.getLine(), loc.getColumn()));
    current_node = nd;
    nd->setOperator(BinaryOperator::getOpcodeStr(E->getOpcode()).str());
    SVisitor::TraverseStmt(E->getLHS());
    nd->setLHS(current_node);
    current_node = nd;
    SVisitor::TraverseStmt(E->getRHS());
    nd->setRHS(current_node);
    current_node = nd;
    //E->dumpColor();
  if (BinaryOperator::getOpcodeStr(E->getOpcode()).str() == "%") {
    std::cout << nd->toCCode() << std::endl;
  }
  }
  return true;
}
bool SVisitor::TraverseBinaryOperator(BinaryOperator *op) {
  return handleBinaryOperator(op);
}

#define GENERAL_BINOP_FALLBACK(NAME, BINOP_TYPE)                               \
  bool SVisitor::TraverseBin##NAME(BINOP_TYPE *E) {                            \
    return handleBinaryOperator(E);                                            \
  }
#define OPERATOR(NAME) GENERAL_BINOP_FALLBACK(NAME, BinaryOperator)
BINOP_LIST()
#undef OPERATOR

#define OPERATOR(NAME)                                                         \
  GENERAL_BINOP_FALLBACK(NAME##Assign, CompoundAssignOperator)
CAO_LIST()
#undef GENERAL_BINOP_FALLBACK
#undef OPERATOR

bool SVisitor::TraverseDeclRefExpr(DeclRefExpr *E) {
  const ValueDecl *D = E->getDecl();
  PresumedLoc loc = SM.getPresumedLoc(E->getLocation());

  //E->dumpColor();
  current_node = shared_ptr<IdentifierNode>(
      new IdentifierNode(loc.getLine(), loc.getColumn(), "unkownid"));
  if (D) {
    current_node = toNode(ctx, loc, D);
  }

  const NamedDecl *FD = E->getFoundDecl();
  if (FD && D != FD) {
    current_node = toNode(ctx, loc, FD);
  }
  return true;
}
bool SVisitor::TraverseCallExpr(CallExpr *E) {
  const FunctionDecl *F = E->getDirectCallee();
  PresumedLoc loc = SM.getPresumedLoc(F->getLocation());
  //DEBUG;
  shared_ptr<CallNode> call(new CallNode(loc.getLine(), loc.getColumn()));
  current_node = call;
  call->setFunction(shared_ptr<IdentifierNode>(
      new IdentifierNode(loc.getLine(), loc.getColumn(),
                         F->getNameInfo().getName().getAsString())));
  for (auto arg : E->arguments()) {
    if (isa<CXXDefaultArgExpr>(arg)) {
      continue ;
    }
    SVisitor::TraverseStmt(arg);
    call->addArg(current_node);
    current_node = call;
  }
  return true;
}

bool SVisitor::TraverseCUDAKernelCallExpr(CUDAKernelCallExpr * E) {
  
  const FunctionDecl *F = E->getDirectCallee();
  PresumedLoc loc = SM.getPresumedLoc(F->getLocation());
  DEBUG;
  shared_ptr<CUDACallNode> call(new CUDACallNode(loc.getLine(), loc.getColumn()));
  current_node = call;
  call->setFunction(shared_ptr<IdentifierNode>(
      new IdentifierNode(loc.getLine(), loc.getColumn(),
                         F->getNameInfo().getName().getAsString())));
  auto config = E->getConfig();

  for (unsigned i = 0, e = config->getNumArgs(); i != e; ++i) {
    auto conf = config->getArg(i);
    if (isa<CXXDefaultArgExpr>(conf)) {
      continue ;
    }
    //conf->dumpColor();
    SVisitor::TraverseStmt(conf);

    call->addConfig(current_node);
    current_node = call;
  }
  for (auto arg : E->arguments()) {
    if (isa<CXXDefaultArgExpr>(arg)) {
      continue ;
    }
    SVisitor::TraverseStmt(arg);
    call->addArg(current_node);
    current_node = call;
  }
  return true;
}

bool SVisitor::TraverseIntegerLiteral(IntegerLiteral *E) {
  PresumedLoc loc = SM.getPresumedLoc(E->getExprLoc());
  current_node = toNode(ctx, loc, E->getValue());
  return true;
}
bool SVisitor::TraverseCharacterLiteral(CharacterLiteral *E) {
  PresumedLoc loc = SM.getPresumedLoc(E->getExprLoc());
  current_node = shared_ptr<CharacterNode>(
      new CharacterNode(loc.getLine(), loc.getColumn(), E->getValue()));
  return true;
}

bool SVisitor::TraverseStringLiteral(StringLiteral *E) {
  // cout << "striiiing" << endl;
  PresumedLoc loc = SM.getPresumedLoc(E->getExprLoc());
  current_node = shared_ptr<StringNode>(
      new StringNode(loc.getLine(), loc.getColumn(), E->getString().str()));
  return true;
}

bool SVisitor::TraverseCXXBindTemporaryExpr(CXXBindTemporaryExpr *E) {
  auto temp = E->getSubExpr();
  PresumedLoc loc = SM.getPresumedLoc(E->getExprLoc());
  TraverseStmt(temp);
  return true;
}
bool SVisitor::TraverseCXXConstructExpr(CXXConstructExpr *E) {

  if (E->getNumArgs() == 1) {
    return TraverseStmt(E->getArg(0));
  } else {
    PresumedLoc loc = SM.getPresumedLoc(E->getExprLoc());

  shared_ptr<CompoundNode> nd(new CompoundNode(loc.getLine(), loc.getColumn()));    
    for (unsigned i = 0, e = E->getNumArgs(); i != e; ++i) {
      if (isa<CXXDefaultArgExpr>(E->getArg(i))) {
        continue ;
      }
      current_node = nd;
      nd->isListInitialization(true);
      SVisitor::TraverseStmt(E->getArg(i));
      *nd <<= current_node;
    }
    current_node = nd;
  }
  return true;
}

bool SVisitor::TraverseMaterializeTemporaryExpr(MaterializeTemporaryExpr *nd) {
  // nd->dumpColor();
  TraverseStmt(nd->GetTemporaryExpr());
  return true;
}

bool SVisitor::TraverseImplicitCastExpr(ImplicitCastExpr *nd) {
  TraverseStmt(nd->getSubExpr());
  return true;
}

bool SVisitor::TraverseMemberExpr(MemberExpr *nd) {
  //nd->dumpColor();
  PresumedLoc loc = SM.getPresumedLoc(nd->getExprLoc());
  shared_ptr<MemberNode> member = shared_ptr<MemberNode>(
      new MemberNode(loc.getLine(), loc.getColumn()));
  current_node = member;
  SVisitor::TraverseStmt(nd->getBase());
  assert(current_node != member);
  member->setLHS(current_node);
  current_node = member;
  current_node = toNode(ctx,loc, nd->getMemberDecl());
  assert(current_node != member);
  member->setRHS(current_node);
  current_node = member;
  return true;
}

bool SVisitor::TraverseParenExpr(ParenExpr *E) {
  PresumedLoc loc = SM.getPresumedLoc(E->getExprLoc());
  TraverseStmt(E->getSubExpr());
  current_node = shared_ptr<ParenNode>(
      new ParenNode(loc.getLine(), loc.getColumn(), current_node));
  return true;
}
bool SVisitor::TraverseNullStmt(NullStmt * stmt) {
  PresumedLoc loc = SM.getPresumedLoc(stmt->getLocStart());
  current_node = shared_ptr<SkipStmtNode>(
    new SkipStmtNode(loc.getLine(), loc.getColumn()));
  return true;
}
void SVisitor::addCurrent() {
  *prog_ <<= current_node;
  current_node = prog_;
}
