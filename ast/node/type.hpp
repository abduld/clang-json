

#ifndef __TYPE_H__
#define __TYPE_H__

class TypeNode : public Node, public NodeAcceptor<TypeNode> {
public:
  TypeNode(const int &row, const int &col, const int &endrow, const int &endcol,
           const string &raw)
      : Node(row, col, endrow, endcol, raw) {}
  TypeNode(const int &row, const int &col, const int &endrow, const int &endcol,
           const string &raw, const string &typ)
      : Node(row, col, endrow, endcol, raw) {
    addBase(typ);
  }
  void addQualifyer(const string &qual) {
    addQualifyer(shared_ptr<SymbolNode>(
        new SymbolNode(row_, col_, endrow_, endcol_, qual, qual)));
  }
  void addQualifyer(const shared_ptr<Node> &qual) {
    qualifiers_.push_back(qual);
  }
  void addBase(const string &base) {
    addBase(shared_ptr<SymbolNode>(
        new SymbolNode(row_, col_, endrow_, endcol_, base, base)));
  }
  void addBase(const shared_ptr<Node> &base) { base_.push_back(base); }
  void addAddressSpace(const string &addr) {
    addAddressSpace(shared_ptr<SymbolNode>(
        new SymbolNode(row_, col_, endrow_, endcol_, addr, addr)));
  }
  void addAddressSpace(const shared_ptr<Node> &addr) {
    address_space_.push_back(addr);
  }
  vector<shared_ptr<Node>> getQualifiers() const { return qualifiers_; }
  vector<shared_ptr<Node>> getBase() const { return base_; }
  vector<shared_ptr<Node>> getAddressSpace() const { return address_space_; }
  virtual Json toEsprima_() override {
    Json::object obj;
    vector<Json> addrs, quals, bases;
    obj["type"] = "TypeSpecification";
    for (auto addr : address_space_) {
      addrs.push_back(addr->toEsprima_());
    }
    for (auto qual : qualifiers_) {
      quals.push_back(qual->toEsprima_());
    }
    for (auto base : base_) {
      bases.push_back(base->toEsprima_());
    }
    obj["loc"] = getLocation();
    obj["raw"] = raw_;
    obj["cform"] = toCCode();
    obj["address_spaces"] = addrs;
    obj["qualifiers"] = quals;
    obj["bases"] = bases;
    return obj;
  }

  virtual void toCCode_(ostringstream &o) override {
    for (auto addr : address_space_) {
      addr->toCCode_(o);
      if (addr != *address_space_.end()) {
        o << " ";
      }
    }
    for (auto qual : qualifiers_) {
      qual->toCCode_(o);
      if (qual != *qualifiers_.end()) {
        o << " ";
      }
    }
    for (auto base : base_) {
      base->toCCode_(o);
      if (base != *base_.end()) {
        o << " ";
      }
    }
  }
  virtual void toString_(ostringstream &o) override { toCCode_(o); }
  virtual void toJSON_(ostringstream &o) { o << "{\"type\": \"unknown\"}"; }
  virtual string getHead() const override { return head_; }
  void traverse(ASTVisitor *visitor) override {
    for (auto addr : address_space_) {
      addr->traverse(visitor);
    }
    for (auto qual : qualifiers_) {
      qual->traverse(visitor);
    }
    for (auto base : base_) {
      base->traverse(visitor);
    }
  }

private:
  string head_ = "Type";
  vector<shared_ptr<Node>> qualifiers_{};
  vector<shared_ptr<Node>> base_{};
  vector<shared_ptr<Node>> address_space_{};
};
#endif /* __TYPE_H__ */
