#pragma once
#include "ScriptObject.h"
#include "VbRuntime.h"
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <libxslt/xsltInternals.h>

namespace vb { namespace runtime {

class VbXmlDocument;
class VbXmlNode : public ScriptObject
{
public:
	VB_INTERFACE()

	VbXmlNode(VbXmlDocument* docRef, xmlNodePtr node, bool owner);
	~VbXmlNode();

	xmlNodePtr GetNode() { return m_node; }
	void Invalidate(bool removeRef = false);
	void AssertValid();

	VbXmlDocument* GetDocRef() { return m_docRef; }
	xmlNodePtr InsertNode(VbXmlNode* nodeRef, VbXmlNode* nodeBeforeRef = nullptr);
	void UnlinkNode();

	int GetAttributeCount();
	void GetAttribute(int index, Variant& result);
	void GetAttribute(const CString& name, Variant& result);

	int GetChildCount();
	void GetChild(int index, Variant& result);

	void GetNextSibling(Variant& result);

	void vbsGetNodeType(vb::CallContext& ctx);
	void vbsGetBaseName(vb::CallContext& ctx);
	void vbsGetNodeName(vb::CallContext& ctx);
	void vbsGetName(vb::CallContext& ctx);
	void vbsGetPrefix(vb::CallContext& ctx);
	void vbsGetNamespaceURI(vb::CallContext& ctx);
	void vbsGetOwnerDocument(vb::CallContext& ctx);
	void vbsGetParentNode(vb::CallContext& ctx);
	void vbsGetFirstChild(vb::CallContext& ctx);
	void vbsGetLastChild(vb::CallContext& ctx);
	void vbsGetNextSibling(vb::CallContext& ctx);
	void vbsGetPreviousSibling(vb::CallContext& ctx);
	void vbsGetAttributes(vb::CallContext& ctx);
	void vbsGetChildNodes(vb::CallContext& ctx);
	void vbsGetText(vb::CallContext& ctx);
	void vbsSetText(vb::CallContext& ctx);
	void vbsGetNodeValue(vb::CallContext& ctx);
	void vbsGetXml(vb::CallContext& ctx);

	void vbsAppendChild(vb::CallContext& ctx);
	void vbsCloneNode(vb::CallContext& ctx);
	void vbsHasChildNodes(vb::CallContext& ctx);
	void vbsInsertBefore(vb::CallContext& ctx);
	void vbsRemoveChild(vb::CallContext& ctx);
	void vbsSelectNodes(vb::CallContext& ctx);
	void vbsSelectSingleNode(vb::CallContext& ctx);
	void vbsGetAttribute(vb::CallContext& ctx);
	void vbsSetAttribute(vb::CallContext& ctx);
	void vbsRemoveAttribute(vb::CallContext& ctx);

protected:
	VbXmlNode() {}  // needed for VbXmlDocument constructor
	void SetDocRef(VbXmlDocument* docRef);
	void InvalidateNodeList(xmlNodePtr cur);
	void ClearOwnership() { m_isNodeOwner = false; }
	friend class VbXmlDocument;

protected:
	xmlNodePtr m_node{ nullptr };
	VbXmlDocument* m_docRef{ nullptr };  // weak reference (ref count is not affected)
	bool m_isNodeOwner{ false };         // true for 'dangling' nodes only (without parent)
};

class VbXmlParseError;
class VbXmlDocument : public VbXmlNode
{
public:
	VB_INTERFACE()

	VbXmlDocument();
	VbXmlDocument(xmlDocPtr doc);
	~VbXmlDocument();
		
	xmlDocPtr GetDoc() { return m_doc; }

	VbXmlNode* CreateNodeRef(xmlNodePtr node);
	VbXmlNode* GetNodeRef(xmlNodePtr node);
	void RemoveNodeRef(VbXmlNode* nodeRef);
	VbXmlNode* RemoveNodeRef(xmlNodePtr node);
	void AttachNodeRef(VbXmlNode* nodeRef);

	xmlXPathContextPtr NewXPathContext();

	static void vbsNewObject(vb::CallContext& ctx);

	void vbsGetDocumentElement(vb::CallContext& ctx);
	void vbsGetParseError(vb::CallContext& ctx);

	void vbsGetPreserveWhiteSpace(vb::CallContext& ctx);
	void vbsSetPreserveWhiteSpace(vb::CallContext& ctx);

	void vbsLoadXml(vb::CallContext& ctx);
	void vbsCreateNode(vb::CallContext& ctx);
	void vbsCreateElement(vb::CallContext& ctx);
	void vbsGetProperty(vb::CallContext& ctx);
	void vbsSetProperty(vb::CallContext& ctx);

private:
	void FreeDoc();
	void AttachNodeRef(VbXmlDocument* sourceDocRef, xmlNodePtr cur, bool sameForSiblings);
	xmlNsPtr AllocTempNamespace(const CString& prefix, const CString& uri);

private:
	xmlDocPtr m_doc;
	VbXmlParseError* m_parseError;
	CGrowingMap<xmlNodePtr, xmlNodePtr, VbXmlNode*, VbXmlNode*> m_mapNodeRefs;  // alive ScriptObjects created for VB
	CMap<CStringA, LPCSTR, CStringA, CStringA> m_selectNamespaces;
	CString m_propNamespaces;
	xmlNsPtr m_nsList{ nullptr };
	bool m_preserveSpaces{ false };
};

class VbXmlParseError : public ScriptObject
{
public:
	VB_INTERFACE()

	VbXmlParseError() {}
	
	void Set(xmlErrorPtr error);
	void Set(int errorCode, const CString& text);
	void Clear();

	void vbsGetErrorCode(vb::CallContext& ctx);
	void vbsGetLine(vb::CallContext& ctx);
	void vbsGetLinePos(vb::CallContext& ctx);
	void vbsGetReason(vb::CallContext& ctx);

private:
	int m_errorCode{ 0 }, m_line{ 0 }, m_linePos{ 0 };
	CString m_reasonText;
};

class VbXmlNodeList;
class VbXmlNodeListEnumerator : public VbEnumerator
{
public:
	VB_INTERFACE()

	VbXmlNodeListEnumerator(VbXmlNodeList* nodeList);
	virtual ~VbXmlNodeListEnumerator();

	// Inherited via VbEnumerator
	virtual bool Next(Variant& value) override;

private:
	VbXmlNodeList* m_nodeList;
	int m_index{ 0 };
	VbXmlNode* m_lastNodeRef{ nullptr };
};

class VbXmlNodeList : public ScriptObject, public VbEnumerable
{
public:
	VB_INTERFACE()

	VbXmlNodeList(VbXmlNode* node, bool attributes);
	VbXmlNodeList(VbXmlDocument* doc, xmlNodePtr* nodeArray, int count);
	virtual ~VbXmlNodeList();

	// Inherited via VbEnumerable
	virtual VbEnumerator* CreateEnumerator() override;

	int GetItemCount() { return (int)m_nodes.GetSize(); }
	VbXmlNode* GetItem(int i);
	VbXmlNode* GetParentNodeRef() { return m_nodeRef; }
	bool IsAttributeList() { return m_isAttributeList; }

	void vbsGetLength(vb::CallContext& ctx);
	void vbsGetItem(vb::CallContext& ctx);

	void vbsGetNamedItem(vb::CallContext& ctx);  // available when m_isAttributeList is true
	void vbsSetNamedItem(vb::CallContext& ctx);  // available when m_isAttributeList is true
	
private:
	bool m_isAttributeList{ false };
	int m_index{ 0 };
	CArray<VbXmlNode*> m_nodes;
	VbXmlNode* m_nodeRef{ nullptr };
};

class VbXmlSchema : public ScriptObject
{
public:
	VB_INTERFACE()

	VbXmlSchema();
	~VbXmlSchema();

	static void vbsNewObject(vb::CallContext& ctx);

	void vbsGetParseError(vb::CallContext& ctx);
	void vbsLoadXml(vb::CallContext& ctx);
	void vbsValidate(vb::CallContext& ctx);

private:
	void FreeSchema();

private:
	xmlSchemaPtr m_schema;
	xmlDocPtr m_schemaDoc;
	VbXmlParseError* m_parseError;
};

class VbXsltProcessor : public ScriptObject
{
public:
	VB_INTERFACE()

	VbXsltProcessor();
	~VbXsltProcessor();

	void AddErrorText(const CString& errorText);

	static void vbsNewObject(vb::CallContext& ctx);

	void vbsGetParseError(vb::CallContext& ctx);
	
	void vbsSetParameter(vb::CallContext& ctx);

	void vbsLoadXml(vb::CallContext& ctx);
	void vbsTransform(vb::CallContext& ctx);

private:
	void FreeStylesheet();

private:
	xsltStylesheetPtr m_stylesheet;
	VbXmlParseError* m_parseError;
	CString m_errorText;
	CMap<CStringA, LPCSTR, CStringA, CStringA> m_parameters;
};

}}
