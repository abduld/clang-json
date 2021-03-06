
#ifndef __CHARLIT_H__
#define __CHARLIT_H__

class CharacterNode : public AtomNode<unsigned char>,
                      public NodeAcceptor<CharacterNode> {
public:
  CharacterNode(const int &row, const int &col, const int &endrow,
                const int &endcol, const string &raw)
      : AtomNode<unsigned char>(row, col, endrow, endcol, raw) {}
  CharacterNode(const int &row, const int &col, const int &endrow,
                const int &endcol, const string &raw, const unsigned char &v)
      : AtomNode<unsigned char>(row, col, endrow, endcol, raw, v) {}
  string getHead() const override { return head_; }
  void toCCode_(ostringstream &o) override {
    o << "'" << string(1, getConstant()) << "'";
  }
  void traverse(ASTVisitor *visitor) override { accept(visitor); }

  virtual Json toEsprima_() override {
    Json::object obj;
    obj["type"] = Json("CharLiteral");
    obj["loc"] = getLocation();
    obj["raw"] = raw_;
    obj["cform"] = toCCode();
    obj["value"] = getConstant();
    return obj;
  }

private:
  string head_ = "Character";
};

#endif /* __CHARLIT_H__ */
