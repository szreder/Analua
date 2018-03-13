#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

#include "EnumHelpers.hpp"
#include "ValueType.hpp"

class Node {
public:
	enum class Type {
		Chunk,
		ExprList,
		VarList,
		ParamList,
		Ellipsis,
		LValue,
		FunctionCall,
		MethodCall,
		Assignment,
		Value,
		TableCtor,
		Field,
		BinOp,
		UnOp,
		Break,
		Return,
		Function,
		If,
		While,
		Repeat,
		For,
		ForEach,
		_last,
	};

	virtual void append(Node *n) { assert(false); }

	virtual void print(int indent = 0) const
	{
		do_indent(indent);
		std::cout << "Node\n";
	}

	Node() = default;
	virtual ~Node() = default;
	Node(Node &&) = default;

	virtual bool isValue() const { return false; }

	Node(const Node &) = delete;
	Node & operator = (const Node &) = delete;
	Node & operator = (Node &&) = default;

	virtual Type type() const = 0;

protected:
	void do_indent(int indent) const
	{
		for (int i = 0; i < indent; ++i)
			std::cout << '\t';
	}
};

class Chunk : public Node {
public:
	void append(Node *n) override { m_children.emplace_back(n); }

	const std::vector <std::unique_ptr <Node> > & children() const { return m_children; }

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "Chunk:\n";
		for (const auto &n : m_children)
			n->print(indent + 1);
	}

	Node::Type type() const override { return Type::Chunk; }

private:
	std::vector <std::unique_ptr <Node> > m_children;
};

class ParamList : public Node {
public:
	ParamList() : m_ellipsis{false} {}

	void append(const std::string &name) { m_names.push_back(name); }
	void append(std::string &&name) { m_names.push_back(std::move(name)); }

	void setEllipsis() { m_ellipsis = true; };


	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "Name list: ";
		if (!m_names.empty())
			std::cout << m_names.front();
		if (m_names.size() > 1) {
			for (auto name = m_names.cbegin() + 1; name != m_names.cend(); ++name)
				std::cout << ", " << *name;
		}
		if (m_ellipsis)
			std::cout << "...";
		std::cout << '\n';
	}

	const std::vector <std::string> & names() const { return m_names; }
	Node::Type type() const override { return Type::ParamList; }

private:
	std::vector <std::string> m_names;
	bool m_ellipsis;
};

class ExprList : public Node {
public:
	void append(Node *n) override { m_exprs.emplace_back(n); }

	const std::vector <std::unique_ptr <Node> > & exprs() const { return m_exprs; }

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "Expression list: [\n";
		for (const auto &n : m_exprs)
			n->print(indent + 1);
		do_indent(indent);
		std::cout << "]\n";
	}

	Node::Type type() const override { return Type::ExprList; }

private:
	std::vector <std::unique_ptr <Node> > m_exprs;
};

class LValue : public Node {
public:
	enum class Type {
		Bracket,
		Dot,
		Name,
	};

	LValue(Node *tableExpr, Node *keyExpr) : m_type{Type::Bracket}, m_tableExpr{tableExpr}, m_keyExpr{keyExpr} {}
	LValue(Node *tableExpr, const std::string &fieldName) : m_type{Type::Dot}, m_tableExpr{tableExpr}, m_name{fieldName} {}
	LValue(Node *tableExpr, std::string &&fieldName) : m_type{Type::Dot}, m_tableExpr{tableExpr}, m_name{std::move(fieldName)} {}
	LValue(const std::string &varName) : m_type{Type::Name}, m_name{varName} {}
	LValue(std::string &&varName) : m_type{Type::Name}, m_name{std::move(varName)} {}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "LValue";
		switch (m_type) {
			case Type::Bracket:
				std::cout << " bracket operator:\n";
				m_tableExpr->print(indent + 1);
				m_keyExpr->print(indent + 1);
				break;
			case Type::Dot:
				std::cout << " dot operator:\n";
				m_tableExpr->print(indent + 1);
				do_indent(indent + 1);
				std::cout << "Field name: " << m_name << '\n';
				break;
			case Type::Name:
				std::cout << '\n';
				do_indent(indent + 1);
				std::cout << m_name << '\n';
				break;
		}
	}

	const std::string & name() const { return m_name; }
	const Node * tableExpr() const { return m_tableExpr.get(); }
	const Node * keyExpr() const { return m_keyExpr.get(); }

	Type lvalueType() const { return m_type; }
	Node::Type type() const override { return Node::Type::LValue; }
private:
	Type m_type;
	std::unique_ptr <Node> m_tableExpr;
	std::unique_ptr <Node> m_keyExpr;
	std::string m_name;
};

class VarList : public Node {
public:
	void append(LValue *lval)
	{
		m_vars.emplace_back(lval);
	}

	const std::vector <std::unique_ptr <LValue> > & vars() const
	{
		return m_vars;
	}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "Variable list: [\n";
		for (const auto &lv : m_vars) {
			lv->print(indent + 1);
		}
		do_indent(indent);
		std::cout << "]\n";
	}

	Node::Type type() const override { return Type::VarList; }

private:
	std::vector <std::unique_ptr <LValue> > m_vars;
};

class Ellipsis : public Node {
public:
	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "Ellipsis (...)\n";
	}

	Node::Type type() const override { return Type::Ellipsis; }
};

class Assignment : public Node {
public:
	Assignment(VarList *vl, ExprList *el) : m_varList{vl}, m_exprList{el}, m_local{false} {}
	Assignment(ParamList *pl, ExprList *el) : m_varList{new VarList{}}, m_exprList{el}, m_local{true}
	{
		for (const auto &name : pl->names())
			m_varList->append(new LValue{name});
		delete pl;
	}

	const VarList * varList() const { return m_varList.get(); }
	const ExprList * exprList() const { return m_exprList.get(); }

	void print(int indent = 0) const override
	{
		do_indent(indent);
		if (m_local)
			std::cout << "local ";
		std::cout << "assignment:\n";
		m_varList->print(indent + 1);
		if (m_exprList) {
			m_exprList->print(indent + 1);
		} else {
			do_indent(indent + 1);
			std::cout << "nil\n";
		}
	}

	Node::Type type() const override { return Type::Assignment; }

private:
	std::unique_ptr <VarList> m_varList;
	std::unique_ptr <ExprList> m_exprList;
	bool m_local;
};

class Value : public Node {
public:
	bool isValue() const override { return true; }

	Node::Type type() const override { return Type::Value; }

	virtual ValueType valueType() const = 0;
};

class NilValue : public Value {
public:
	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "nil\n";
	}

	ValueType valueType() const override { return ValueType::Nil; }
};

class BooleanValue : public Value {
public:
	BooleanValue(bool v) : m_value{v} {}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << std::boolalpha << m_value << '\n';
	}

	ValueType valueType() const override { return ValueType::Boolean; }

	bool value() const { return m_value; }
private:
	bool m_value;
};

class StringValue : public Value {
public:
	StringValue(const std::string &v) : m_value{v} {}
	StringValue(std::string &&v) : m_value{std::move(v)} {}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "String: " << m_value << '\n';
	}

	ValueType valueType() const override { return ValueType::String; }

	const std::string & value() const { return m_value; }
private:
	std::string m_value;
};

class IntValue : public Value {
public:
	constexpr IntValue(long v) : m_value{v} {}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "Int: " << m_value << '\n';
	}

	ValueType valueType() const override { return ValueType::Integer; }

	long value() const { return m_value; }
private:
	long m_value;
};

class RealValue : public Value {
public:
	constexpr RealValue(double v) : m_value{v} {}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "Real: " << m_value << '\n';
	}

	ValueType valueType() const override { return ValueType::Real; }

	double value() const { return m_value; }
private:
	double m_value;
};

class FunctionCall : public Node {
public:
	FunctionCall(Node *funcExpr, ExprList *args) : m_functionExpr{funcExpr}, m_args{args} {}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "Function call:\n";
		m_functionExpr->print(indent + 1);
		do_indent(indent);
		std::cout << "Args:\n";
		m_args->print(indent + 1);
	}

	const Node * functionExpr() const { return m_functionExpr.get(); }

	const ExprList * args() const { return m_args.get(); }

	Node::Type type() const override { return Type::FunctionCall; }
private:
	std::unique_ptr <Node> m_functionExpr;
	std::unique_ptr <ExprList> m_args;
};

class MethodCall : public FunctionCall {
public:
	MethodCall(Node *funcExpr, ExprList *args, const std::string &methodName) : FunctionCall{funcExpr, args}, m_methodName{methodName} {}
	MethodCall(Node *funcExpr, ExprList *args, std::string &&methodName) : FunctionCall{funcExpr, args}, m_methodName{std::move(methodName)} {}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "Method call:\n";
		functionExpr()->print(indent + 1);
		do_indent(indent);
		std::cout << "Method name: " << m_methodName << '\n';
		args()->print(indent + 1);
	}

	Node::Type type() const override { return Type::MethodCall; }
private:
	std::string m_methodName;
};

class Field : public Node {
public:
	enum class Type {
		Brackets,
		Literal,
		NoIndex,
	};

	Field(Node *expr, Node *val) : m_type{Type::Brackets}, m_keyExpr{expr}, m_valueExpr{val} {}
	Field(const std::string &s, Node *val) : m_type{Type::Literal}, m_fieldName{s}, m_valueExpr{val} {}
	Field(Node *val) : m_type{Type::NoIndex}, m_keyExpr{nullptr}, m_valueExpr{val} {}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		switch (m_type) {
			case Type::Brackets:
				std::cout << "Expr to expr:\n";
				m_keyExpr->print(indent + 1);
				break;
			case Type::Literal:
				std::cout << "Name to expr:\n";
				do_indent(indent + 1);
				std::cout << m_fieldName << '\n';
				break;
			case Type::NoIndex:
				std::cout << "Expr:\n";
				break;
		}

		m_valueExpr->print(indent + 1);
	}

	Type fieldType() const { return m_type; }

	Node::Type type() const override { return Node::Type::Field; }

	const std::string & fieldName() const { return m_fieldName; }
	const Node * keyExpr() const { return m_keyExpr.get(); }
	const Node * valueExpr() const { return m_valueExpr.get(); }

private:
	Type m_type;
	std::string m_fieldName;
	std::unique_ptr <Node> m_keyExpr;
	std::unique_ptr <Node> m_valueExpr;
};

class TableCtor : public Node {
public:
	void append(Field *f) { m_fields.emplace_back(f); }

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "Table:\n";
		for (const auto &p : m_fields)
			p->print(indent + 1);
	}

	const std::vector <std::unique_ptr <Field> > & fields() const { return m_fields; }

	Node::Type type() const override { return Node::Type::TableCtor; }
private:
	std::vector <std::unique_ptr <Field> > m_fields;
};

class BinOp : public Node {
public:
	enum class Type {
		Or,
		And,
		Equal,
		NotEqual,
		Less,
		LessEqual,
		Greater,
		GreaterEqual,
		Concat,
		Plus,
		Minus,
		Times,
		Divide,
		Modulo,
		Exponentation,
		_last
	};

	BinOp(Type t, Node *left, Node *right) : m_type{t}, m_left{left}, m_right{right} {}

	static const std::vector <ValueType> & applicableTypes(Type t)
	{
		static auto ApplicableTypes = []{
			std::array <std::vector <ValueType>, toUnderlying(Type::_last)> result;

			for (auto v : {Type::Plus, Type::Minus, Type::Times, Type::Divide})
				result[toUnderlying(v)] = {ValueType::Integer, ValueType::Real};

			result[toUnderlying(Type::Modulo)] = {ValueType::Integer};

			return result;
		}();

		return ApplicableTypes[toUnderlying(t)];
	}

	static bool isApplicable(Type t, ValueType vt)
	{
		auto types = applicableTypes(t);
		return std::find(types.begin(), types.end(), vt) != types.end();
	}

	static const char * toString(Type t)
	{
		static const char *s[] = {"or", "and", "==", "~=", "<", "<=", ">", ">=", "..", "+", "-", "*", "/", "%", "^"};
		return s[toUnderlying(t)];
	}

	Type binOpType() const { return m_type; }

	const Node * left() const { return m_left.get(); }
	const Node * right() const { return m_right.get(); }

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "BinOp: " << toString() << '\n';
		m_left->print(indent + 1);
		m_right->print(indent + 1);
	}

	Node::Type type() const override { return Node::Type::BinOp; }

	const char * toString() const { return toString(m_type); }

private:
	Type m_type;
	std::unique_ptr <Node> m_left;
	std::unique_ptr <Node> m_right;
};

class UnOp : public Node {
public:
	enum class Type {
		Negate,
		Not,
		Length,
	};

	UnOp(Type t, Node *op) : m_type{t}, m_operand{op} {}

	static const char * toString(Type t)
	{
		static const char *s[] = {"-", "not", "#"};
		return s[toUnderlying(t)];
	}

	Type unOpType() const { return m_type; }

	Node * operand() const { return m_operand.get(); }

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "UnOp: " << toString() << '\n';
		m_operand->print(indent + 1);
	}

	Node::Type type() const override { return Node::Type::UnOp; }

	const char * toString() const { return toString(m_type); }

private:
	Type m_type;
	std::unique_ptr <Node> m_operand;
};

class Break : public Node {
public:
	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "break\n";
	}

	Node::Type type() const override { return Node::Type::Break; }
};

class Return : public Node {
public:
	Return(ExprList *exprList) : m_exprList{exprList} {}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "return\n";
		if (m_exprList)
			m_exprList->print(indent + 1);
	}

	Node::Type type() const override { return Node::Type::Return; }

private:
	std::unique_ptr <ExprList> m_exprList;
};

typedef std::pair <std::vector <std::string>, std::string> FunctionName;

class Function : public Node {
public:
	Function(ParamList *params, Chunk *chunk) : m_params{params}, m_chunk{chunk}, m_local{false} {}

	void setLocal() { m_local = true; }

	void setName(const FunctionName &name)
	{
		m_name = name.first;
		m_method = name.second;
	}

	void setName(const std::string &name)
	{
		m_name.clear();
		m_name.push_back(name);
	}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		if (m_local)
			std::cout << "local ";
		std::cout << "function ";

		if (!m_name.empty()) {
			std::cout << m_name[0];
			for (auto iter = m_name.begin() + 1; iter != m_name.end(); ++iter)
				std::cout << "." << *iter;

			if (!m_method.empty())
				std::cout << ":" << m_method;
			std::cout << '\n';
		} else {
			std::cout << "<anonymous>\n";
		}

		do_indent(indent);
		std::cout << "params:\n";
		if (m_params) {
			m_params->print(indent + 1);
		} else {
			do_indent(indent + 1);
			std::cout << "<no params>\n";
		}

		do_indent(indent);
		std::cout << "body:\n";
		if (m_chunk) {
			m_chunk->print(indent + 1);
		} else {
			do_indent(indent + 1);
			std::cout << "<empty>\n";
		}
	}

	Node::Type type() const override { return Node::Type::Function; }

private:
	std::vector <std::string> m_name;
	std::string m_method;
	std::unique_ptr <ParamList> m_params;
	std::unique_ptr <Chunk> m_chunk;
	bool m_local;
};

class If : public Node {
public:
	If(Node *condition, Chunk *chunk) : m_condition{condition}, m_chunk{chunk} {}

	void setElse(Chunk *chunk) { m_else.reset(chunk); }
	void setNextIf(If *next) { m_nextIf.reset(next); }

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "if:\n";
		m_condition->print(indent + 1);
		if (m_chunk) {
			m_chunk->print(indent + 1);
		} else {
			do_indent(indent + 1);
			std::cout << "<empty>\n";
		}

		if (m_nextIf) {
			do_indent(indent);
			std::cout << "else:\n";
			m_nextIf->print(indent);
		}

		if (m_else) {
			do_indent(indent);
			std::cout << "else:\n";
			m_else->print(indent + 1);
		}
	}

	Node::Type type() const override { return Node::Type::If; }

private:
	std::unique_ptr <Node> m_condition;
	std::unique_ptr <Node> m_chunk;
	std::unique_ptr <If> m_nextIf;
	std::unique_ptr <Chunk> m_else;
};

class While : public Node {
public:
	While(Node *condition, Chunk *chunk) : m_condition{condition}, m_chunk{chunk} {}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "while:\n";
		m_condition->print(indent + 1);
		m_chunk->print(indent + 1);
	}

	Node::Type type() const override { return Node::Type::While; }

private:
	std::unique_ptr <Node> m_condition;
	std::unique_ptr <Chunk> m_chunk;
};

class Repeat : public Node {
public:
	Repeat(Node *condition, Chunk *chunk) : m_condition{condition}, m_chunk{chunk} {}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "repeat:\n";
		m_chunk->print(indent + 1);
		m_condition->print(indent + 1);
	}

	Node::Type type() const override { return Node::Type::Repeat; }

private:
	std::unique_ptr <Node> m_condition;
	std::unique_ptr <Chunk> m_chunk;
};

class For : public Node {
public:
	For(const std::string &iterator, Node *start, Node *limit, Node *step, Chunk *chunk)
		: m_iterator{iterator}, m_start{start}, m_limit{limit}, m_step{step}, m_chunk{chunk} {}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "for:\n";

		do_indent(indent);
		std::cout << "iterator: " << m_iterator << '\n';

		do_indent(indent);
		std::cout << "start:\n";
		m_start->print(indent + 1);

		do_indent(indent);
		std::cout << "limit:\n";
		m_limit->print(indent + 1);

		if (m_step) {
			do_indent(indent);
			std::cout << "step:\n";
			m_step->print(indent + 1);
		}

		do_indent(indent);
		std::cout << "do:\n";
		m_chunk->print(indent + 1);
	}

	Node::Type type() const override { return Node::Type::For; }

private:
	std::string m_iterator;
	std::unique_ptr <Node> m_start, m_limit, m_step;
	std::unique_ptr <Chunk> m_chunk;
};

class ForEach : public Node {
public:
	ForEach(ParamList *iterators, ExprList *exprs, Chunk *chunk)
		: m_iterators{iterators}, m_exprs{exprs}, m_chunk{chunk} {}

	void print(int indent = 0) const override
	{
		do_indent(indent);
		std::cout << "for_each:\n";
		m_iterators->print(indent + 1);

		do_indent(indent);
		std::cout << "in:\n";
		m_exprs->print(indent + 1);

		do_indent(indent);
		std::cout << "do:\n";
		m_chunk->print(indent + 1);
	}

	Node::Type type() const override { return Node::Type::ForEach; }

private:
	std::unique_ptr <ParamList> m_iterators;
	std::unique_ptr <ExprList> m_exprs;
	std::unique_ptr <Chunk> m_chunk;
};
