#include "VbXmlDocument.h"


#include <libxml/xpath.h>
#include <libxml/parser.h>
#include <libxml/xmlsave.h>
#include <libxml/xpathInternals.h>
//#include <libxslt/transform.h>
//#include <libxslt/xsltutils.h>

// Use as a param to function (does not outlive a function call)
#define XML2STR(s) (LPCTSTR)CA2CT((const char*)s, 65001)

// Use as a param to function (does not outlive a function call)
#define STR2XML(s) (const xmlChar*)(const char*)CT2CA(s, 65001)

// Use to hold the result in the current scope
#define STR2XML_VAR(varName, str) \
	CT2AEX<> varName ## _holder(str, 65001); \
	const xmlChar* varName = (const xmlChar*)(const char*)varName ## _holder;

//
//
//static void BuildNodeText(xmlNodePtr cur, CString& result, int& firstCdataStart, int& lastCdataEnd)
//{
//	switch (cur->type)
//	{
//	case XML_CDATA_SECTION_NODE:
//		if (firstCdataStart < 0)
//			firstCdataStart = result.GetLength();
//		result.Append(XML2STR(cur->content));
//		lastCdataEnd = result.GetLength();
//		break;
//
//	case XML_TEXT_NODE:
//		result.Append(XML2STR(cur->content));
//		break;
//
//	case XML_DOCUMENT_NODE:
//	case XML_DOCUMENT_FRAG_NODE:
//	case XML_ELEMENT_NODE:
//	case XML_ATTRIBUTE_NODE:
//	{
//		xmlNodePtr tmp = cur->children;
//		while (tmp != NULL)
//		{
//			BuildNodeText(tmp, result, firstCdataStart, lastCdataEnd);
//			tmp = tmp->next;
//		}
//		break;
//	}
//
//	case XML_ENTITY_REF_NODE:
//	{
//		xmlEntityPtr ent = xmlGetDocEntity(cur->doc, cur->name);
//		if (ent == NULL)
//			return;
//
//		xmlNodePtr tmp = ent->children;
//		while (tmp)
//		{
//			BuildNodeText(tmp, result, firstCdataStart, lastCdataEnd);
//			tmp = tmp->next;
//		}
//		break;
//	}
//	}
//}

static void XMLCDECL XmlCommonErrorHandler(void* ctx, const char* msg, ...)
{
	// do nothing - it prevents printing error text to console

#if defined(DEBUG) && defined(_MSC_VER)
	va_list args;
	va_start(args, msg);
	CStringA s;
	s.FormatV(msg, args);
	va_end(args);
	TRACE("%s\n", s.GetString());
#endif
}

// taken from libxml and 'allowDefault' parameter added
static xmlNsPtr xmlNewReconciledNs(xmlDocPtr doc, xmlNodePtr tree, xmlNsPtr ns, bool allowDefault)
{
	xmlNsPtr def;
	xmlChar prefix[50];
	int counter = 1;

	/*
	 * Search an existing namespace definition inherited.
	 */
	def = xmlSearchNsByHref(doc, tree, ns->href);
	if (def != NULL && (allowDefault || def->prefix != nullptr))
		return(def);

	/*
	 * Find a close prefix which is not already in use.
	 * Let's strip namespace prefixes longer than 20 chars !
	 */
	if (ns->prefix == NULL)
		snprintf((char*)prefix, sizeof(prefix), "default");
	else
		snprintf((char*)prefix, sizeof(prefix), "%.20s", (char*)ns->prefix);

	def = xmlSearchNs(doc, tree, prefix);
	while (def != NULL) {
		if (counter > 1000) return(NULL);
		if (ns->prefix == NULL)
			snprintf((char*)prefix, sizeof(prefix), "default%d", counter++);
		else
			snprintf((char*)prefix, sizeof(prefix), "%.20s%d",
				(char*)ns->prefix, counter++);
		def = xmlSearchNs(doc, tree, prefix);
	}

	/*
	 * OK, now we are ready to create a new one.
	 */
	def = xmlNewNs(tree, ns->href, prefix);
	return(def);
}

namespace vb { namespace runtime {

//static VbXmlNode* GetNodeArg(const Variant& v, bool notNull = true)
//{
//	if (v.type != Variant::Type::ScriptObject)
//		throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid argument: XML node was expected."));
//
//	if (v.objValue == nullptr)
//	{
//		if (notNull)
//			throw VbRuntimeException(VbErrorNumber::NullObject, _T("Invalid argument: Null object reference."));
//		return nullptr;
//	}
//
//	VbXmlNode* result = dynamic_cast<VbXmlNode*>(v.objValue);
//	if (result == nullptr)
//		throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid argument: XML node was expected."));
//
//	result->AssertValid();
//	return result;
//}
//static VbXmlDocument* GetDocumentArg(const Variant& v, bool notNull = true)
//{
//	if (v.type != Variant::Type::ScriptObject)
//		throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid argument: XML node was expected."));
//
//	if (v.objValue == nullptr)
//	{
//		if (notNull)
//			throw VbRuntimeException(VbErrorNumber::NullObject, _T("Invalid argument: Null object reference."));
//		return nullptr;
//	}
//
//	VbXmlDocument* result = dynamic_cast<VbXmlDocument*>(v.objValue);
//	if (result == nullptr)
//		throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid argument: XML node was expected."));
//	if (result->GetDoc() == nullptr)
//		throw VbRuntimeException(VbErrorNumber::General, _T("Invalid XML document."));
//	return result;
//}

////////////////////////////////////////////////////////////////////

VbXmlNode::VbXmlNode(VbXmlDocument* docRef, xmlNodePtr node, bool owner)
{
	m_docRef = docRef;
	m_node = node;
	m_isNodeOwner = owner;

	if (m_docRef != this)
		m_docRef->AddRef();  // hold a reference to the document
}

VbXmlNode::~VbXmlNode()
{
	Invalidate(true);
}

void VbXmlNode::AssertValid()
{
//	if (m_node == nullptr)
//		throw VbRuntimeException(VbErrorNumber::General, _T("Invalid XML node."));
}

void VbXmlNode::SetDocRef(VbXmlDocument* docRef)
{
	// must not be a document itself
	ASSERT(m_docRef != this);
	ASSERT(docRef != this);

	if (m_docRef)
		m_docRef->Release();
	m_docRef = docRef;
	m_docRef->AddRef();
}

void VbXmlNode::Invalidate(bool removeRef)
{
	if (removeRef && m_docRef)
	{
		m_docRef->RemoveNodeRef(this);
	}
	if (m_isNodeOwner && m_node)
	{
		if (m_node->children && m_node->type != XML_ENTITY_REF_NODE)
			InvalidateNodeList(m_node->children);

		if ((m_node->type == XML_ELEMENT_NODE || m_node->type == XML_XINCLUDE_START || m_node->type == XML_XINCLUDE_END) && m_node->properties)
			InvalidateNodeList((xmlNodePtr)m_node->properties);

		xmlFreeNode(m_node);
	}
	if (m_docRef && m_docRef != this)
		m_docRef->Release();
	m_docRef = nullptr;
	m_node = nullptr;
}

void VbXmlNode::InvalidateNodeList(xmlNodePtr cur)
{
	while (cur)
	{
		if (cur->children && cur->type != XML_ENTITY_REF_NODE)
			InvalidateNodeList(cur->children);

		if ((cur->type == XML_ELEMENT_NODE || cur->type == XML_XINCLUDE_START || cur->type == XML_XINCLUDE_END) && cur->properties)
			InvalidateNodeList((xmlNodePtr)cur->properties);

		VbXmlNode* nodeRef = m_docRef->RemoveNodeRef(cur);
		if (nodeRef)
			nodeRef->Invalidate();
		cur = cur->next;
	}
}

void VbXmlNode::vbsGetNodeType(vb::CallContext& ctx)
{
	AssertValid();
	*ctx.pReturnValue = (int)m_node->type;
}

void VbXmlNode::vbsGetBaseName(vb::CallContext& ctx)
{
	AssertValid();
	*ctx.pReturnValue = XmlCharsToString(m_node->name);
}

void VbXmlNode::vbsGetNodeName(vb::CallContext& ctx)
{
	AssertValid();
	CString result;
	if ((m_node->type == XML_ATTRIBUTE_NODE || m_node->type == XML_ELEMENT_NODE) && m_node->ns)
		result.Format(_T("%s:%s"), XML2STR(m_node->ns->prefix), XML2STR(m_node->name));
	else
		result = XmlCharsToString(m_node->name);
	*ctx.pReturnValue = result;
}

void VbXmlNode::vbsGetName(vb::CallContext& ctx)
{
	AssertValid();

	if (m_node->type != XML_ATTRIBUTE_NODE)
		throw VbRuntimeException(VbErrorNumber::PropertyOrMethodNotFound, _T("Property 'Name' is not supported for this node type."));

	CString result;
	if (m_node->ns)
		result.Format(_T("%s:%s"), XML2STR(m_node->ns->prefix), XML2STR(m_node->name));
	else
		result = XmlCharsToString(m_node->name);
	*ctx.pReturnValue = result;
}

void VbXmlNode::vbsGetPrefix(vb::CallContext& ctx)
{
	AssertValid();
	CString result;
	if ((m_node->type == XML_ATTRIBUTE_NODE || m_node->type == XML_ELEMENT_NODE) && m_node->ns)
		result = XmlCharsToString(m_node->ns->prefix);
	*ctx.pReturnValue = result;
}

void VbXmlNode::vbsGetNamespaceURI(vb::CallContext& ctx)
{
	AssertValid();
	CString result;
	if ((m_node->type == XML_ATTRIBUTE_NODE || m_node->type == XML_ELEMENT_NODE) && m_node->ns)
		result = XmlCharsToString(m_node->ns->href);
	*ctx.pReturnValue = result;
}

void VbXmlNode::vbsGetOwnerDocument(vb::CallContext& ctx)
{
	AssertValid();
	if (this == m_docRef)
		*ctx.pReturnValue = (ScriptObject*)nullptr;
	else
		*ctx.pReturnValue = static_cast<ScriptObject*>(m_docRef);
}

void VbXmlNode::vbsGetParentNode(vb::CallContext& ctx)
{
	AssertValid();
	ctx.pReturnValue->Attach(m_docRef->CreateNodeRef(m_node->parent));
}

void VbXmlNode::vbsGetFirstChild(vb::CallContext& ctx)
{
	AssertValid();
	ctx.pReturnValue->Attach(m_docRef->CreateNodeRef(m_node->children));
}

void VbXmlNode::vbsGetLastChild(vb::CallContext& ctx)
{
	AssertValid();
	ctx.pReturnValue->Attach(m_docRef->CreateNodeRef(m_node->last));
}

void VbXmlNode::vbsGetNextSibling(vb::CallContext& ctx)
{
	GetNextSibling(*ctx.pReturnValue);
}

void VbXmlNode::vbsGetPreviousSibling(vb::CallContext& ctx)
{
	AssertValid();
	ctx.pReturnValue->Attach(m_docRef->CreateNodeRef(m_node->prev));
}

void VbXmlNode::vbsGetAttributes(vb::CallContext& ctx)
{
	AssertValid();
	if (m_node->type != XML_ELEMENT_NODE)
		throw VbRuntimeException(VbErrorNumber::General, _T("XML node is not an element."));

	ctx.pReturnValue->Attach(new VbXmlNodeList(this, true));

}

void VbXmlNode::vbsGetChildNodes(vb::CallContext& ctx)
{
	AssertValid();
	ctx.pReturnValue->Attach(new VbXmlNodeList(this, false));
}

void VbXmlNode::vbsGetText(vb::CallContext& ctx)
{
	AssertValid();

	CString result;
	int startCdata = -1, endCdata = -1;
	BuildNodeText(m_node, result, startCdata, endCdata);

	// trim right till the last CDATA
	while (endCdata < result.GetLength() && result.GetLength() > 0)
	{
		TCHAR c = result[result.GetLength() - 1];
		if (c == ' ' || c == '\t' || (c >= 10 && c <= 13))
			result.Truncate(result.GetLength() - 1);
		else
			break;
	}

	// trim left to the first CDATA
	if (startCdata < 0)
		startCdata = result.GetLength();

	int spaceCount = 0;
	while (spaceCount < startCdata)
	{
		TCHAR c = result[spaceCount];
		if (c == ' ' || c == '\t' || (c >= 10 && c <= 13))
			spaceCount++;
		else
			break;
	}
	if (spaceCount > 0)
		result.Delete(0, spaceCount);

	*ctx.pReturnValue = result;
}

void VbXmlNode::vbsSetText(vb::CallContext& ctx)
{
	AssertValid();

	CString text = GetString(*ctx.pParams[0]);

	switch (m_node->type)
	{
	case XML_DOCUMENT_FRAG_NODE:
	case XML_ELEMENT_NODE:
	case XML_ATTRIBUTE_NODE:
	{
		// clear node
		if (m_node->children)
		{
			InvalidateNodeList(m_node->children);
			xmlFreeNodeList(m_node->children);
			m_node->children = m_node->last = NULL;
		}

		xmlNodePtr nodeText = xmlNewDocText(m_docRef->GetDoc(), STR2XML(text));
		if (nodeText)
			xmlAddChild(m_node, nodeText);
		break;
	}
	case XML_CDATA_SECTION_NODE:
	case XML_TEXT_NODE:
	case XML_COMMENT_NODE:
		if (
			m_node->content && m_node->content != (xmlChar*)&(m_node->properties) &&
			(m_node->doc == NULL || m_node->doc->dict == NULL || xmlDictOwns(m_node->doc->dict, m_node->content) == 0)
		)
			xmlFree(m_node->content);
		m_node->content = xmlStrdup(STR2XML(text));
		m_node->properties = NULL;
		break;

	default:
		throw VbRuntimeException(VbErrorNumber::General, _T("Setting Text is not supported for this type of node."));
	}
}

void VbXmlNode::vbsGetNodeValue(vb::CallContext& ctx)
{
	AssertValid();

	CString result;
	switch (m_node->type)
	{
	case XML_ATTRIBUTE_NODE:
		if (m_node->children)
		{
			if (m_node->children->next == nullptr && m_node->children->type == XML_TEXT_NODE)  // optimization for the common case
				result = XmlCharsToString(m_node->children->content);
			else
			{
				auto tmp = xmlNodeGetContent(m_node);
				result = XmlCharsToString(tmp);
				xmlFree(tmp);
			}
		}
		break;

	case XML_CDATA_SECTION_NODE:
	case XML_TEXT_NODE:
	case XML_COMMENT_NODE:
		result = XmlCharsToString(m_node->content);
		break;

	default:
		ctx.pReturnValue->SetNull();
		break;
	}

	*ctx.pReturnValue = result;
}

void VbXmlNode::vbsGetXml(vb::CallContext& ctx)
{
	AssertValid();

	// the node can contain undeclared namespace - declare it temporary to have complete XML
	xmlNsPtr nsDef_Saved = m_node->nsDef;
	xmlNsPtr tempNs = nullptr;
	_xmlNs temp = { 0 };
	if (m_node->type == XML_ELEMENT_NODE && m_node->ns)
	{
		xmlNsPtr ns = m_node->nsDef;
		while (ns)
		{
			if (xmlStrEqual(ns->prefix, m_node->ns->prefix) && xmlStrEqual(ns->href, m_node->ns->href))
				break;
			ns = ns->next;
		}

		// if declaration not found - make it
		if (ns == nullptr)
		{
			tempNs = &temp;
			tempNs->type = m_node->ns->type;
			tempNs->prefix = m_node->ns->prefix;
			tempNs->href = m_node->ns->href;
			tempNs->next = m_node->nsDef;
			m_node->nsDef = tempNs;
		}
	}

	xmlBufferPtr buf = xmlBufferCreate();
	xmlSaveCtxtPtr saveCtx = xmlSaveToBuffer(buf, "UTF-8", XML_SAVE_AS_XML | XML_SAVE_NO_DECL);
	auto resultCode = xmlSaveTree(saveCtx, m_node);
	if (resultCode < 0)
	{
		xmlSaveClose(saveCtx);
		m_node->nsDef = nsDef_Saved;  // restore ns declaration
		throw VbRuntimeException(VbErrorNumber::General, _T("Failed to dump XML."));
	}
	xmlSaveFlush(saveCtx);

	//xmlNodeDump(buf, m_docRef->GetDoc(), m_node, 0, 0);
	CString result = XmlCharsToString(xmlBufferContent(buf), xmlBufferLength(buf));
	xmlSaveClose(saveCtx);
	m_node->nsDef = nsDef_Saved;  // restore ns declaration

	*ctx.pReturnValue = result;
}

xmlNodePtr VbXmlNode::InsertNode(VbXmlNode* nodeRef, VbXmlNode* nodeBeforeRef)
{
	if (nodeBeforeRef && nodeBeforeRef->GetNode()->parent != m_node)
		throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("'Before' node is not a child node."));

	nodeRef->UnlinkNode();

	bool reconcileNs = false;
	if (GetDocRef() != nodeRef->GetDocRef())
	{
		auto wrapCtx = xmlDOMWrapNewCtxt();
		int wrapResult = xmlDOMWrapAdoptNode(wrapCtx, nodeRef->GetDocRef()->GetDoc(), nodeRef->GetNode(), m_docRef->GetDoc(), m_node, 0);
		xmlDOMWrapFreeCtxt(wrapCtx);
		if (wrapResult != 0)
			throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid XML node for adoption into another document."));

		m_docRef->AttachNodeRef(nodeRef);
	}
	else
	{
		reconcileNs = true;
	}

	xmlNodePtr newChild;
	if (nodeBeforeRef == nullptr)
		newChild = xmlAddChild(m_node, nodeRef->GetNode());
	else
		newChild = xmlAddPrevSibling(nodeBeforeRef->GetNode(), nodeRef->GetNode());
	if (newChild == nullptr)
		throw VbRuntimeException(VbErrorNumber::General, _T("Failed to add child node."));

	if (reconcileNs)
	{
		// declare and replace 'dangling' namespaces (including temporary ones created by VbXmlDocument::AllocTempNamespace)
		if (newChild->type == XML_ATTRIBUTE_NODE)
		{
			if (newChild->ns)
			{
				newChild->ns = xmlNewReconciledNs(m_docRef->GetDoc(), m_node, newChild->ns, false);
			}
		}
		else
		{
			// xmlReconciliateNs declares default namespace with prefix 'default' - fix it before xmlReconciliateNs call
			if (
				// default namespace
				newChild->ns && newChild->ns->prefix == nullptr &&
				// no declaration found
				xmlSearchNsByHref(m_docRef->GetDoc(), newChild, newChild->ns->href) == nullptr
			)
			{
				// if current node belongs to the same namespace, declare it here
				if (m_node->ns && xmlStrEqual(m_node->ns->href, newChild->ns->href))
				{
					// xmlReconciliateNs will link created namespace declaration to appended subtree
					m_node->ns = xmlNewNs(m_node, m_node->ns->href, m_node->ns->prefix);
				}
				else
				{
					// declare default namespace in the child node
					newChild->ns = xmlNewNs(newChild, newChild->ns->href, nullptr);
				}
			}
			xmlReconciliateNs(m_docRef->GetDoc(), newChild);
		}
	}

	nodeRef->ClearOwnership();
	if (newChild != nodeRef->GetNode())
	{
		// the node was merged and freed by libxml
		nodeRef->Invalidate(true);
	}

	return newChild;
}

void VbXmlNode::UnlinkNode()
{
	xmlUnlinkNode(m_node);
	m_isNodeOwner = true;
}

int VbXmlNode::GetAttributeCount()
{
	AssertValid();

	if (m_node->type != XML_ELEMENT_NODE)
		return 0;

	int count = 0;
	for (auto n = m_node->properties; n != nullptr; n = n->next)
		count++;

	return count;
}

void VbXmlNode::GetAttribute(int index, Variant& result)
{
	AssertValid();

	xmlNodePtr node = nullptr;
	if (m_node->type == XML_ELEMENT_NODE)
	{
		int i = 0;
		xmlAttrPtr n;
		for (n = m_node->properties; i < index && n != nullptr; n = n->next)
			i++;

		if (i == index)
			node = (xmlNodePtr)n;
	}

	result.Attach(m_docRef->CreateNodeRef(node));
}

void VbXmlNode::GetAttribute(const CString& name, Variant& result)
{
	AssertValid();

	if (m_node->type != XML_ELEMENT_NODE)
		throw VbRuntimeException(VbErrorNumber::General, _T("XML node is not an element."));

	result.Attach(m_docRef->CreateNodeRef((xmlNodePtr)xmlHasProp(m_node, STR2XML(name))));
}

int VbXmlNode::GetChildCount()
{
	AssertValid();

	int count = 0;
	for (auto n = m_node->children; n != nullptr; n = n->next)
		count++;
		
	return count;
}

void VbXmlNode::GetChild(int index, Variant& result)
{
	AssertValid();

	int i = 0;
	xmlNodePtr n;
	for (n = m_node->children; i < index && n != nullptr; n = n->next)
		i++;

	xmlNodePtr node = nullptr;
	if (i == index)
		node = (xmlNodePtr)n;

	result.Attach(m_docRef->CreateNodeRef(node));
}

void VbXmlNode::GetNextSibling(Variant& result)
{
	AssertValid();
	result.Attach(m_docRef->CreateNodeRef(m_node->next));
}

void VbXmlNode::vbsAppendChild(vb::CallContext& ctx)
{
	AssertValid();
	VbXmlNode* node = GetNodeArg(*ctx.pParams[0]);

	if (node->GetNode()->type == XML_ATTRIBUTE_NODE)
		throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Attribute is not allowed in this method."));

	ctx.pReturnValue->Attach(m_docRef->CreateNodeRef(InsertNode(node)));
}

void VbXmlNode::vbsCloneNode(vb::CallContext& ctx)
{
	AssertValid();
	bool deep = GetBool(*ctx.pParams[0]);
	xmlNodePtr node = xmlDocCopyNode(m_node, m_docRef->GetDoc(), deep ? 1 : 0);
	if (node->type == XML_DOCUMENT_NODE)
	{
		ASSERT(node->doc == (xmlDocPtr)node);
		ctx.pReturnValue->Attach(new VbXmlDocument((xmlDocPtr)node));
	}
	else
		ctx.pReturnValue->Attach(m_docRef->CreateNodeRef(node));
}

void VbXmlNode::vbsHasChildNodes(vb::CallContext& ctx)
{
	AssertValid();
	*ctx.pReturnValue = (bool)(m_node->children != nullptr);
}

void VbXmlNode::vbsInsertBefore(vb::CallContext& ctx)
{
	AssertValid();
	VbXmlNode* node = GetNodeArg(*ctx.pParams[0]);
	VbXmlNode* nodeBefore = GetNodeArg(*ctx.pParams[1], false);

	if (node->GetNode()->type == XML_ATTRIBUTE_NODE)
		throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Attribute is not allowed in this method."));

	ctx.pReturnValue->Attach(m_docRef->CreateNodeRef(InsertNode(node, nodeBefore)));
}

void VbXmlNode::vbsRemoveChild(vb::CallContext& ctx)
{
	AssertValid();
	VbXmlNode* nodeRef = GetNodeArg(*ctx.pParams[0]);

	if (nodeRef->GetNode()->parent != m_node)
		throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("XML node is not the child node."));

	nodeRef->UnlinkNode();
	*ctx.pReturnValue = static_cast<ScriptObject*>(nodeRef);
}

void VbXmlNode::vbsSelectNodes(vb::CallContext& ctx)
{
	AssertValid();
	CString xpath = GetString(*ctx.pParams[0]);

	auto xpathCtx = m_docRef->NewXPathContext();
	if (xmlXPathSetContextNode(m_node, xpathCtx) != 0)
	{
		xmlXPathFreeContext(xpathCtx);
		throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid node for XPath search."));
	}

	auto xpathResult = xmlXPathEvalExpression(STR2XML(xpath), xpathCtx);
	xmlXPathFreeContext(xpathCtx);
	if (xpathResult == NULL)
	{
		CString s;
		s.Format(_T("Invalid XPath expression %s."), xpath.GetString());
		throw VbRuntimeException(VbErrorNumber::InvalidArgument, s);
	}

	auto noteSet = xpathResult->nodesetval;
	ctx.pReturnValue->Attach(new VbXmlNodeList(m_docRef, noteSet ? noteSet->nodeTab : nullptr, noteSet ? noteSet->nodeNr : 0));
	xmlXPathFreeObject(xpathResult);
}

void VbXmlNode::vbsSelectSingleNode(vb::CallContext& ctx)
{
	AssertValid();
	CString xpath = GetString(*ctx.pParams[0]);

	auto xpathCtx = m_docRef->NewXPathContext();
	if (xmlXPathSetContextNode(m_node, xpathCtx) != 0)
	{
		xmlXPathFreeContext(xpathCtx);
		throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid node for XPath search."));
	}

	auto xpathResult = xmlXPathEvalExpression(STR2XML(xpath), xpathCtx);
	xmlXPathFreeContext(xpathCtx);
	if (xpathResult == NULL)
	{
		CString s;
		s.Format(_T("Invalid XPath expression %s."), xpath.GetString());
		throw VbRuntimeException(VbErrorNumber::InvalidArgument, s);
	}

	if (xpathResult->nodesetval == nullptr || xpathResult->nodesetval->nodeNr == 0)
		*ctx.pReturnValue = (ScriptObject*)nullptr;
	else
		ctx.pReturnValue->Attach(m_docRef->CreateNodeRef(xpathResult->nodesetval->nodeTab[0]));
	xmlXPathFreeObject(xpathResult);
}

void VbXmlNode::vbsGetAttribute(vb::CallContext& ctx)
{
	AssertValid();
	CString name = GetString(*ctx.pParams[0]);

	if (m_node->type != XML_ELEMENT_NODE)
		throw VbRuntimeException(VbErrorNumber::General, _T("XML node is not an element."));

	xmlAttrPtr prop = xmlHasProp(m_node, STR2XML(name));
	if (prop == nullptr)
		ctx.pReturnValue->SetNull();
	else
	{
		CString result;
		if (prop->children->next == nullptr && prop->children->type == XML_TEXT_NODE)  // optimization for the common case
			result = XmlCharsToString(prop->children->content);
		else
		{
			auto tmp = xmlNodeGetContent((xmlNodePtr)prop);
			result = XmlCharsToString(tmp);
			xmlFree(tmp);
		}
		*ctx.pReturnValue = result;
	}
}

void VbXmlNode::vbsSetAttribute(vb::CallContext& ctx)
{
	AssertValid();
	CString name = GetString(*ctx.pParams[0]);
	CString value = GetString(*ctx.pParams[1]);

	if (m_node->type != XML_ELEMENT_NODE)
		throw VbRuntimeException(VbErrorNumber::General, _T("XML node is not an element."));

	xmlSetProp(m_node, STR2XML(name), STR2XML(value));
}

void VbXmlNode::vbsRemoveAttribute(vb::CallContext& ctx)
{
	AssertValid();
	CString name = GetString(*ctx.pParams[0]);

	if (m_node->type != XML_ELEMENT_NODE)
		throw VbRuntimeException(VbErrorNumber::General, _T("XML node is not an element."));

	xmlAttrPtr prop = xmlHasProp(m_node, STR2XML(name));
	if (prop == nullptr)
		return;

	auto nodeRef = m_docRef->GetNodeRef((xmlNodePtr)prop);
	if (nodeRef)
		nodeRef->UnlinkNode();
	else
		xmlRemoveProp(prop);
}

#define VB_NODE_API(className) \
VB_DECLARE_GETPROPERTY(className, vbsGetNodeType, "NodeType")                 \
VB_DECLARE_GETPROPERTY(className, vbsGetBaseName, "BaseName")                 \
VB_DECLARE_GETPROPERTY(className, vbsGetNodeName, "NodeName")                 \
VB_DECLARE_GETPROPERTY(className, vbsGetName, "Name")                         \
VB_DECLARE_GETPROPERTY(className, vbsGetPrefix, "Prefix")                     \
VB_DECLARE_GETPROPERTY(className, vbsGetNamespaceURI, "NamespaceURI")         \
VB_DECLARE_GETPROPERTY(className, vbsGetOwnerDocument, "OwnerDocument")       \
VB_DECLARE_GETPROPERTY(className, vbsGetParentNode, "ParentNode")             \
VB_DECLARE_GETPROPERTY(className, vbsGetFirstChild, "FirstChild")             \
VB_DECLARE_GETPROPERTY(className, vbsGetLastChild, "LastChild")               \
VB_DECLARE_GETPROPERTY(className, vbsGetNextSibling, "NextSibling")           \
VB_DECLARE_GETPROPERTY(className, vbsGetPreviousSibling, "PreviousSibling")   \
VB_DECLARE_GETPROPERTY(className, vbsGetAttributes, "Attributes")             \
VB_DECLARE_GETPROPERTY(className, vbsGetChildNodes, "ChildNodes")             \
VB_DECLARE_PROPERTY(className, vbsGetText, vbsSetText, "Text")                \
VB_DECLARE_GETPROPERTY(className, vbsGetNodeValue, "NodeValue")               \
VB_DECLARE_GETPROPERTY(className, vbsGetXml, "Xml")                           \
VB_DECLARE_FUNCTION(className, vbsAppendChild, "AppendChild", 1)	              \
VB_DECLARE_FUNCTION(className, vbsCloneNode, "CloneNode", 1)	                  \
VB_DECLARE_FUNCTION(className, vbsHasChildNodes, "HasChildNodes", 0)          \
VB_DECLARE_FUNCTION(className, vbsInsertBefore, "InsertBefore", 2)            \
VB_DECLARE_FUNCTION(className, vbsRemoveChild, "RemoveChild", 1)              \
VB_DECLARE_FUNCTION(className, vbsSelectNodes, "SelectNodes", 1)              \
VB_DECLARE_FUNCTION(className, vbsSelectSingleNode, "SelectSingleNode", 1)	  \
VB_DECLARE_FUNCTION(className, vbsGetAttribute, "GetAttribute", 1)            \
VB_DECLARE_SUBROUTINE(className, vbsSetAttribute, "SetAttribute", 2)          \
VB_DECLARE_SUBROUTINE(className, vbsRemoveAttribute, "RemoveAttribute", 1)

VB_DECLARE_TYPE(VbXmlNode)
VB_NODE_API(VbXmlNode)

/////////////////////////////////////////////////////////////////////

VbXmlDocument::VbXmlDocument()
{
	m_parseError = new VbXmlParseError();
	m_docRef = this;
	m_doc = xmlNewDoc(nullptr);
	m_node = (xmlNodePtr)m_doc;
}

void VbXmlDocument::AddRef() {}
void VbXmlDocument::Release() {}

VbXmlDocument::VbXmlDocument(xmlDocPtr doc)
{
	m_parseError = new VbXmlParseError();
	m_docRef = this;
	m_doc = doc;
	m_node = (xmlNodePtr)doc;
}

VbXmlDocument::~VbXmlDocument()
{
	m_parseError->Release();
	m_parseError = nullptr;
	FreeDoc();
	m_docRef = nullptr;
}

void VbXmlDocument::FreeDoc()
{
	auto pair = m_mapNodeRefs.PGetFirstAssoc();
	while (pair)
	{
		pair->value->Invalidate();
		pair = m_mapNodeRefs.PGetNextAssoc(pair);
	}
	m_mapNodeRefs.RemoveAll();

	if (m_doc)
		xmlFreeDoc(m_doc);

	xmlFreeNsList(m_nsList);
	m_nsList = nullptr;

	m_doc = nullptr;
	m_node = nullptr;
}

VbXmlNode* VbXmlDocument::CreateNodeRef(xmlNodePtr node)
{
	if (node == nullptr)
		return nullptr;

	ASSERT(m_doc == node->doc);
	if (m_doc != node->doc)
		return nullptr;

	if (node == m_node)
	{
		AddRef();
		return this;
	}

	VbXmlNode* result;
	auto pair = m_mapNodeRefs.PLookup(node);
	if (pair)
	{
		result = pair->value;
		result->AddRef();
	}
	else
	{
		result = new VbXmlNode(this, node, node->type != XML_DOCUMENT_NODE && node->parent == nullptr);
		m_mapNodeRefs.SetAt(node, result);
	}
	return result;
}

VbXmlNode* VbXmlDocument::GetNodeRef(xmlNodePtr node)
{
	auto pair = m_mapNodeRefs.PLookup(node);
	if (pair)
		return pair->value;

	return nullptr;
}

void VbXmlDocument::RemoveNodeRef(VbXmlNode* nodeRef)
{
	m_mapNodeRefs.RemoveKey(nodeRef->GetNode());
}

VbXmlNode* VbXmlDocument::RemoveNodeRef(xmlNodePtr node)
{
	auto pPair = m_mapNodeRefs.PLookup(node);
	if (pPair == nullptr)
		return nullptr;

	VbXmlNode* nodeRef = pPair->value;
	m_mapNodeRefs.RemoveKey(node);
	return nodeRef;
}

void VbXmlDocument::AttachNodeRef(VbXmlDocument* sourceDocRef, xmlNodePtr cur, bool sameForSiblings)
{
	while (cur)
	{
		if (cur->children && cur->type != XML_ENTITY_REF_NODE)
			AttachNodeRef(sourceDocRef, cur->children, true);

		if ((cur->type == XML_ELEMENT_NODE || cur->type == XML_XINCLUDE_START || cur->type == XML_XINCLUDE_END) && cur->properties)
			AttachNodeRef(sourceDocRef, (xmlNodePtr)cur->properties, true);

		auto nodeRef = sourceDocRef->RemoveNodeRef(cur);
		if (nodeRef)
		{
			m_mapNodeRefs.SetAt(cur, nodeRef);
			nodeRef->SetDocRef(this);
		}

		if (!sameForSiblings)
			break;
		cur = cur->next;
	}
}

// Returns temporary xmlNsPtr (freed by VbXmlDocument destructor).
// Attention: temporary xmlNsPtr must never be saved to xmlNode::nsDef (namespace declaration), only to xmlNode::ns,
// because xmlNode::nsDef is freed by libxml.
xmlNsPtr VbXmlDocument::AllocTempNamespace(const CString& prefix, const CString& uri)
{
	STR2XML_VAR(xmlPrefix, prefix);
	STR2XML_VAR(xmlUri, uri);

	if (xmlPrefix[0] == 0)
		xmlPrefix = nullptr;

	xmlNsPtr ns = m_nsList;
	while (ns)
	{
		if (xmlStrEqual(xmlPrefix, ns->prefix) && xmlStrEqual(xmlUri, ns->href))
			break;
		ns = ns->next;
	}
	if (ns)
		return ns;

	ns = xmlNewNs(nullptr, xmlUri, xmlPrefix);
	ns->next = m_nsList;
	m_nsList = ns;
	return ns;
}

xmlXPathContextPtr VbXmlDocument::NewXPathContext()
{
	auto xpathCtx = xmlXPathNewContext(m_docRef->GetDoc());
	if (xpathCtx == NULL)
		throw VbRuntimeException(VbErrorNumber::InternalError, _T("Failed to create XPath context."));

	auto pair = m_selectNamespaces.PGetFirstAssoc();
	while (pair)
	{
		const char* prefix = pair->key.GetString();
		const char* uri = pair->value.GetString();
		if (xmlXPathRegisterNs(xpathCtx, (const xmlChar*)prefix, (const xmlChar*)uri) != 0)
		{
			xmlXPathFreeContext(xpathCtx);

			CString s;
			s.Format(_T("Failed to register namespace (prefix=%s, uri=%s)."), XML2STR(prefix), XML2STR(uri));
			throw VbRuntimeException(VbErrorNumber::InvalidAttribute, s);
		}
		pair = m_selectNamespaces.PGetNextAssoc(pair);
	}
	return xpathCtx;
}

void VbXmlDocument::AttachNodeRef(VbXmlNode* nodeRef)
{
	ASSERT(nodeRef && nodeRef->GetNode() && nodeRef->GetDocRef());

	if (nodeRef->GetDocRef() == this)
		return;

	AttachNodeRef(nodeRef->GetDocRef(), nodeRef->GetNode(), false);
}

void VbXmlDocument::vbsNewObject(vb::CallContext& ctx)
{
	ctx.pReturnValue->Attach(new VbXmlDocument());
}

void VbXmlDocument::vbsGetDocumentElement(vb::CallContext& ctx)
{
	AssertValid();
	ctx.pReturnValue->Attach(CreateNodeRef(m_doc->children));
}

void VbXmlDocument::vbsGetParseError(vb::CallContext& ctx)
{
	*ctx.pReturnValue = m_parseError;
}

void VbXmlDocument::vbsGetPreserveWhiteSpace(vb::CallContext& ctx)
{
	*ctx.pReturnValue = m_preserveSpaces;
}

void VbXmlDocument::vbsSetPreserveWhiteSpace(vb::CallContext& ctx)
{
	m_preserveSpaces = GetBool(*ctx.pParams[0]);
}

void VbXmlDocument::vbsLoadXml(vb::CallContext& ctx)
{
	STR2XML_VAR(xml, GetString(*ctx.pParams[0]));

	auto parserCtx = xmlCreateDocParserCtxt(xml);
	if (parserCtx->sax)
	{
		parserCtx->sax->warning = XmlCommonErrorHandler;
		parserCtx->sax->error = XmlCommonErrorHandler;
	}
	xmlCtxtUseOptions(parserCtx, XML_PARSE_NONET | XML_PARSE_IGNORE_ENC | (m_preserveSpaces ? 0 : XML_PARSE_NOBLANKS));
	int result = xmlParseDocument(parserCtx);
	if (result == 0)
	{
		m_parseError->Clear();
		FreeDoc();

		m_doc = parserCtx->myDoc;
		m_node = (xmlNodePtr)m_doc;
	}
	else
		m_parseError->Set(xmlGetLastError());

	xmlFreeParserCtxt(parserCtx);

	*ctx.pReturnValue = (result == 0);
}

void VbXmlDocument::vbsCreateNode(vb::CallContext& ctx)
{
	int type = GetInteger(*ctx.pParams[0]);
	CString name = GetString(*ctx.pParams[1]);
	CString namespaceURI = GetString(*ctx.pParams[2]);

	// if namespace specified name can be in format 'prefix:basename'
	CString prefix;
	xmlNsPtr ns = nullptr;
	if (type == XML_ELEMENT_NODE || type == XML_ATTRIBUTE_NODE)
	{
		int pos = name.Find(_T(':'));
		if (pos >= 0)
		{
			if (namespaceURI.IsEmpty())
				throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid argument 'name': prefix specified but 'namespaceURI' is empty."));

			prefix = name.Left(pos);
			name.Delete(0, pos + 1);
		}
		if (!namespaceURI.IsEmpty())
			ns = AllocTempNamespace(prefix, namespaceURI);
	}

	VbXmlNode* nodeRef = nullptr;
	switch (type)
	{
	case XML_ELEMENT_NODE:
		if (name.IsEmpty())
			throw VbRuntimeException(VbErrorNumber::General, _T("Element name must not be empty."));
		nodeRef = CreateNodeRef(xmlNewDocNode(m_doc, ns, STR2XML(name), nullptr));
		break;
	case XML_ATTRIBUTE_NODE:
		if (name.IsEmpty())
			throw VbRuntimeException(VbErrorNumber::General, _T("Attribute name must not be empty."));
		nodeRef = CreateNodeRef((xmlNodePtr)xmlNewDocProp(m_doc, STR2XML(name), nullptr));
		if (ns)
			xmlSetNs(nodeRef->GetNode(), ns);
		break;
	case XML_TEXT_NODE:
		nodeRef = CreateNodeRef((xmlNodePtr)xmlNewDocText(m_doc, nullptr));
		break;
	case XML_CDATA_SECTION_NODE:
		nodeRef = CreateNodeRef((xmlNodePtr)xmlNewCDataBlock(m_doc, nullptr, 0));
		break;
	case XML_COMMENT_NODE:
		nodeRef = CreateNodeRef((xmlNodePtr)xmlNewDocComment(m_doc, nullptr));
		break;
	default:
		throw VbRuntimeException(VbErrorNumber::General, _T("Unsupported node type."));
	}

	ctx.pReturnValue->Attach(nodeRef);
}

void VbXmlDocument::vbsCreateElement(vb::CallContext& ctx)
{
	CString name = GetString(*ctx.pParams[0]);
	ctx.pReturnValue->Attach(CreateNodeRef(xmlNewDocNode(m_doc, nullptr, STR2XML(name), nullptr)));
}

void VbXmlDocument::vbsGetProperty(vb::CallContext& ctx)
{
	CString name = GetString(*ctx.pParams[0]);

	if (name == _T("SelectionNamespaces"))
	{
		*ctx.pReturnValue = m_propNamespaces;
	}
	else if (name == _T("SelectionLanguage"))
	{
		*ctx.pReturnValue = _T("XPath");
	}
	else
		throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Unsupported property name."));
}

void VbXmlDocument::vbsSetProperty(vb::CallContext& ctx)
{
	CString name = GetString(*ctx.pParams[0]);

	if (name == _T("SelectionNamespaces"))
	{
		CString value = GetString(*ctx.pParams[1]);

		CMap<CString, LPCTSTR, CString, CString> namespaces;
		CString s = value;
		s.Trim();
		int xmlnsLen = (int)_tcslen(_T("xmlns:"));
		while (!s.IsEmpty())
		{
			if (s.Find(_T("xmlns:")) != 0)
				throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid value for property 'SelectionNamespaces': every item should start with 'xmlns:'."));

			int posSeparator = s.Find(_T('='));
			if (posSeparator < 0)
				throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid value for property 'SelectionNamespaces': no corresponding '='."));

			CString prefix = s.Mid(xmlnsLen, posSeparator - xmlnsLen);
			prefix.TrimRight();
			if (xmlValidateNCName(STR2XML(prefix), 0) != 0)
				throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid value for property 'SelectionNamespaces': invalid prefix."));

			s.Delete(0, posSeparator + 1);
			s.TrimLeft();
			if (s.Find(_T('\'')) != 0)
				throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid value for property 'SelectionNamespaces': string literal for URI not found."));

			posSeparator = s.Find(_T('\''), 1);
			if (posSeparator < 0)
				throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid value for property 'SelectionNamespaces': string literal for URI not found."));

			CString uri = s.Mid(1, posSeparator - 1);
			if (namespaces.PLookup(prefix))
				throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid value for property 'SelectionNamespaces': non-unique prefix."));

			namespaces.SetAt(prefix, uri);

			s.Delete(0, posSeparator + 1);
			if (!s.IsEmpty() && !_istspace(s[0]))
				throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Invalid value for property 'SelectionNamespaces': no whitespace between items."));

			s.TrimLeft();
		}

		// copy to m_selectNamespaces
		m_selectNamespaces.RemoveAll();
		auto pair = namespaces.PGetFirstAssoc();
		while (pair)
		{
			m_selectNamespaces.SetAt((LPCSTR)STR2XML(pair->key), (LPCSTR)STR2XML(pair->value));
			pair = namespaces.PGetNextAssoc(pair);
		}

		m_propNamespaces = value;
	}
	else if (name == _T("SelectionLanguage"))
	{
		CString value = GetString(*ctx.pParams[1]);
		if (value.CompareNoCase(_T("XPath")) != 0)
			throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Unsupported value for property 'SelectionLanguage'."));
	}
	else
		throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Unsupported property name."));
}

VB_DECLARE_TYPE(VbXmlDocument)
VB_DECLARE_CONSTRUCTOR(VbXmlDocument, vbsNewObject)
VB_NODE_API(VbXmlDocument)
VB_DECLARE_GETPROPERTY(VbXmlDocument, vbsGetDocumentElement, "DocumentElement")
VB_DECLARE_GETPROPERTY(VbXmlDocument, vbsGetParseError, "ParseError")
VB_DECLARE_PROPERTY(VbXmlDocument, vbsGetPreserveWhiteSpace, vbsSetPreserveWhiteSpace, "PreserveWhiteSpace")
VB_DECLARE_FUNCTION(VbXmlDocument, vbsLoadXml, "LoadXml", 1)
VB_DECLARE_FUNCTION(VbXmlDocument, vbsCreateNode, "CreateNode", 3)
VB_DECLARE_FUNCTION(VbXmlDocument, vbsCreateElement, "CreateElement", 1)
VB_DECLARE_SUBROUTINE(VbXmlDocument, vbsGetProperty, "GetProperty", 1)
VB_DECLARE_SUBROUTINE(VbXmlDocument, vbsSetProperty, "SetProperty", 2)

/////////////////////////////////////////////////////////////////////

void VbXmlParseError::Set(xmlErrorPtr error)
{
	if (error)
	{
		m_errorCode = error->code;
		m_line = error->line;
		m_linePos = error->int2;
		m_reasonText = Utf8ToCString(error->message);
	}
	else
		Clear();
}

void VbXmlParseError::Set(int errorCode, const CString& text)
{
	Clear();
	m_errorCode = errorCode;
	m_reasonText = text;
}

void VbXmlParseError::Clear()
{
	m_errorCode = 0;
	m_line = -1;
	m_linePos = -1;
	m_reasonText.Empty();
}

void VbXmlParseError::vbsGetErrorCode(vb::CallContext& ctx)
{
	*ctx.pReturnValue = m_errorCode;
}

void VbXmlParseError::vbsGetLine(vb::CallContext& ctx)
{
	*ctx.pReturnValue = m_line;
}

void VbXmlParseError::vbsGetLinePos(vb::CallContext& ctx)
{
	*ctx.pReturnValue = m_linePos;
}

void VbXmlParseError::vbsGetReason(vb::CallContext& ctx)
{
	*ctx.pReturnValue = m_reasonText;
}

VB_DECLARE_TYPE(VbXmlParseError)
VB_DECLARE_GETPROPERTY(VbXmlParseError, vbsGetErrorCode, "ErrorCode")
VB_DECLARE_GETPROPERTY(VbXmlParseError, vbsGetLine, "Line")
VB_DECLARE_GETPROPERTY(VbXmlParseError, vbsGetLinePos, "LinePos")
VB_DECLARE_GETPROPERTY(VbXmlParseError, vbsGetReason, "Reason")

/////////////////////////////////////////////////////////////////////

VbXmlNodeList::VbXmlNodeList(VbXmlNode* node, bool attributes)
{
	m_isAttributeList = attributes;
	m_nodeRef = node;
	m_nodeRef->AddRef();
}

VbXmlNodeList::VbXmlNodeList(VbXmlDocument* doc, xmlNodePtr* nodeArray, int count)
{
	m_isAttributeList = false;
	for (int i = 0; i < count; i++)
		m_nodes.Add(doc->CreateNodeRef(nodeArray[i]));
}

VbXmlNodeList::~VbXmlNodeList()
{
	for (int i = 0; i < m_nodes.GetSize(); i++)
	{
		m_nodes[i]->Release();
	}
	m_nodes.RemoveAll();

	if (m_nodeRef)
	{
		m_nodeRef->Release();
		m_nodeRef = nullptr;
	}
}

VbEnumerator* VbXmlNodeList::CreateEnumerator()
{
	return new VbXmlNodeListEnumerator(this);
}

VbXmlNode* VbXmlNodeList::GetItem(int i)
{
	return m_nodes[i];
}

void VbXmlNodeList::vbsGetLength(vb::CallContext & ctx)
{
	if (m_nodeRef)
		*ctx.pReturnValue = m_isAttributeList ? m_nodeRef->GetAttributeCount() : m_nodeRef->GetChildCount();
	else
		*ctx.pReturnValue = (int)m_nodes.GetSize();
}

void VbXmlNodeList::vbsGetItem(vb::CallContext & ctx)
{
	const Variant& vIndex = *ctx.pParams[0];
	int index = GetInteger(vIndex);

	if (m_nodeRef)
	{
		if (m_isAttributeList)
			m_nodeRef->GetAttribute(index, *ctx.pReturnValue);
		else
			m_nodeRef->GetChild(index, *ctx.pReturnValue);
	}
	else
	{
		if (index < 0 || index >= m_nodes.GetSize())
			*ctx.pReturnValue = (ScriptObject*)nullptr;
		else
			*ctx.pReturnValue = m_nodes[index];
	}
}

void VbXmlNodeList::vbsGetNamedItem(vb::CallContext& ctx)
{
	if (!m_isAttributeList || m_nodeRef == nullptr)
		throw VbRuntimeException(VbErrorNumber::PropertyOrMethodNotFound, _T("Method 'GetNamedItem' is not supported in this type of collection."));

	CString name = GetString(*ctx.pParams[0]);
	m_nodeRef->GetAttribute(name, *ctx.pReturnValue);
}

void VbXmlNodeList::vbsSetNamedItem(vb::CallContext& ctx)
{
	if (!m_isAttributeList || m_nodeRef == nullptr)
		throw VbRuntimeException(VbErrorNumber::PropertyOrMethodNotFound, _T("Method 'SetNamedItem' is not supported in this type of collection."));

	VbXmlNode* node = GetNodeArg(*ctx.pParams[0]);
	if (node->GetNode()->type != XML_ATTRIBUTE_NODE)
		throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Node parameter is not an XML attribute."));

	m_nodeRef->AssertValid();
	ctx.pReturnValue->Attach(m_nodeRef->GetDocRef()->CreateNodeRef(m_nodeRef->InsertNode(node)));
}

VB_DECLARE_TYPE(VbXmlNodeList)
VB_DECLARE_GETPROPERTY(VbXmlNodeList, vbsGetLength, "Length")
VB_DECLARE_FUNCTION_FLAGS(VbXmlNodeList, vbsGetItem, "Item", 1, vb::BaseMemberDescriptor::Type::Default)
VB_DECLARE_FUNCTION(VbXmlNodeList, vbsGetNamedItem, "GetNamedItem", 1)
VB_DECLARE_FUNCTION(VbXmlNodeList, vbsSetNamedItem, "SetNamedItem", 1)

//////////////////////////////////////////////////////////////////////////////////////////

VbXmlNodeListEnumerator::VbXmlNodeListEnumerator(VbXmlNodeList* nodeList)
{
	m_nodeList = nodeList;
	m_nodeList->AddRef();
}

VbXmlNodeListEnumerator::~VbXmlNodeListEnumerator()
{
	if (m_nodeList)
	{
		m_nodeList->Release();
		m_nodeList = nullptr;
	}
	if (m_lastNodeRef)
	{
		m_lastNodeRef->Release();
		m_lastNodeRef = nullptr;
	}
}
bool VbXmlNodeListEnumerator::Next(Variant& value)
{
	VbXmlNode* parentRef = m_nodeList->GetParentNodeRef();
	if (parentRef == nullptr)
	{
		// static array of items - just get one by index
		if (m_index < m_nodeList->GetItemCount())
		{
			value = m_nodeList->GetItem(m_index++);
			return true;
		}
		return false;
	}
	else
	{
		if (m_lastNodeRef == nullptr)
		{
			if (m_nodeList->IsAttributeList())
				parentRef->GetAttribute(0, value);
			else
				parentRef->GetChild(0, value);
		}
		else
		{
			m_lastNodeRef->GetNextSibling(value);
		}

		if (value.IsNothing())
			return false;

		if (m_lastNodeRef)
			m_lastNodeRef->Release();
		m_lastNodeRef = (VbXmlNode*)value.objValue;
		m_lastNodeRef->AddRef();
		return true;
	}
}

VB_DECLARE_TYPE(VbXmlNodeListEnumerator)

//////////////////////////////////////////////////////////////////////////////////////////

VbXmlSchema::VbXmlSchema()
{
	m_schema = nullptr;
	m_schemaDoc = nullptr;
	m_parseError = new VbXmlParseError();
}

VbXmlSchema::~VbXmlSchema()
{
	m_parseError->Release();
	m_parseError = nullptr;
	FreeSchema();
}

void VbXmlSchema::FreeSchema()
{
	xmlSchemaFree(m_schema);
	m_schema = nullptr;
	xmlFreeDoc(m_schemaDoc);
	m_schemaDoc = nullptr;
}

void VbXmlSchema::vbsNewObject(vb::CallContext& ctx)
{
	ctx.pReturnValue->Attach(new VbXmlSchema());
}

void VbXmlSchema::vbsGetParseError(vb::CallContext& ctx)
{
	*ctx.pReturnValue = m_parseError;
}

void VbXmlSchema::vbsLoadXml(vb::CallContext& ctx)
{
	STR2XML_VAR(xml, GetString(*ctx.pParams[0]));

	auto parserCtx = xmlCreateDocParserCtxt(xml);
	if (parserCtx->sax)
	{
		parserCtx->sax->warning = XmlCommonErrorHandler;
		parserCtx->sax->error = XmlCommonErrorHandler;
	}
	xmlCtxtUseOptions(parserCtx, XML_PARSE_NONET | XML_PARSE_IGNORE_ENC | XML_PARSE_NOBLANKS);
	int result = xmlParseDocument(parserCtx);
	if (result == 0)
	{
		m_parseError->Clear();
		FreeSchema();

		m_schemaDoc = parserCtx->myDoc;
		xmlSchemaParserCtxtPtr schemaParserCtx = xmlSchemaNewDocParserCtxt(m_schemaDoc);
		xmlSchemaSetParserErrors(schemaParserCtx, XmlCommonErrorHandler, XmlCommonErrorHandler, NULL);
		m_schema = xmlSchemaParse(schemaParserCtx);
		if (m_schema == nullptr)
		{
			result = -1;
			FreeSchema();
			m_parseError->Set(xmlGetLastError());
		}
		xmlSchemaFreeParserCtxt(schemaParserCtx);
	}
	else
		m_parseError->Set(xmlGetLastError());

	xmlFreeParserCtxt(parserCtx);

	*ctx.pReturnValue = (result == 0);
}

void VbXmlSchema::vbsValidate(vb::CallContext& ctx)
{
	VbXmlDocument* docRef = GetDocumentArg(*ctx.pParams[0]);

	if (m_schema == nullptr)
	{
		m_parseError->Set(-1, _T("No XML schema provided."));
		*ctx.pReturnValue = false;
		return;
	}
	m_parseError->Clear();

	xmlSchemaValidCtxtPtr validCtx = xmlSchemaNewValidCtxt(m_schema);
	xmlSchemaSetValidErrors(validCtx, XmlCommonErrorHandler, XmlCommonErrorHandler, NULL);
	int result = xmlSchemaValidateDoc(validCtx, docRef->GetDoc());
	if (result != 0)
		m_parseError->Set(xmlGetLastError());
	xmlSchemaFreeValidCtxt(validCtx);

	*ctx.pReturnValue = (result == 0);
}

VB_DECLARE_TYPE(VbXmlSchema)
VB_DECLARE_CONSTRUCTOR(VbXmlSchema, vbsNewObject)
VB_DECLARE_GETPROPERTY(VbXmlSchema, vbsGetParseError, "ParseError")
VB_DECLARE_FUNCTION(VbXmlSchema, vbsLoadXml, "LoadXml", 1)
VB_DECLARE_FUNCTION(VbXmlSchema, vbsValidate, "Validate", 1)

//////////////////////////////////////////////////////////////////////////////////////////

static void XMLCDECL XsltTransformError(void* ctx, const char* msg, ...)
{
	// do nothing - it prevents printing error text to console
	va_list args;
	va_start(args, msg);
	CStringA s;
	s.FormatV(msg, args);
	va_end(args);

	CString errorText = XML2STR(s);

	VbXsltProcessor* processor = (VbXsltProcessor*)ctx;
	processor->AddErrorText(errorText);
}

VbXsltProcessor::VbXsltProcessor()
{
	m_stylesheet = nullptr;
	m_parseError = new VbXmlParseError();
}

VbXsltProcessor::~VbXsltProcessor()
{
	m_parseError->Release();
	m_parseError = nullptr;
	FreeStylesheet();
}

void VbXsltProcessor::FreeStylesheet()
{
	xsltFreeStylesheet(m_stylesheet);
	m_stylesheet = nullptr;
}

void VbXsltProcessor::AddErrorText(const CString& errorText)
{
	m_errorText += errorText;
}

void VbXsltProcessor::vbsNewObject(vb::CallContext& ctx)
{
	ctx.pReturnValue->Attach(new VbXsltProcessor());
}

void VbXsltProcessor::vbsGetParseError(vb::CallContext& ctx)
{
	*ctx.pReturnValue = m_parseError;
}

void VbXsltProcessor::vbsSetParameter(vb::CallContext& ctx)
{
	CString name = GetString(*ctx.pParams[0]);
	CString value = GetString(*ctx.pParams[1]);

	m_parameters.SetAt((const char*)STR2XML(name), (const char*)STR2XML(value));
}

void VbXsltProcessor::vbsLoadXml(vb::CallContext& ctx)
{
	STR2XML_VAR(xml, GetString(*ctx.pParams[0]));

	auto parserCtx = xmlCreateDocParserCtxt(xml);
	if (parserCtx->sax)
	{
		parserCtx->sax->warning = XmlCommonErrorHandler;
		parserCtx->sax->error = XmlCommonErrorHandler;
	}
	xmlCtxtUseOptions(parserCtx, XML_PARSE_NONET | XML_PARSE_IGNORE_ENC | XML_PARSE_NOBLANKS);
	int result = xmlParseDocument(parserCtx);
	if (result == 0)
	{
		m_parseError->Clear();
		FreeStylesheet();

		xmlDocPtr doc = parserCtx->myDoc;
		xmlResetLastError();
		m_stylesheet = xsltParseStylesheetDoc(doc);  // to prevent error output to console call xsltSetGenericErrorFunc from main() function
		if (m_stylesheet == nullptr)
		{
			result = -1;
			m_parseError->Set(-1, _T("Failed to process XSLT document."));
			xmlFreeDoc(doc);
			parserCtx->myDoc = nullptr;
		}
	}
	else
		m_parseError->Set(xmlGetLastError());

	xmlFreeParserCtxt(parserCtx);

	*ctx.pReturnValue = (result == 0);
}

void VbXsltProcessor::vbsTransform(vb::CallContext& ctx)
{
	VbXmlDocument* docRef = GetDocumentArg(*ctx.pParams[0]);

	if (m_stylesheet == nullptr)
	{
		m_parseError->Set(-1, _T("No XSLT provided."));
		*ctx.pReturnValue = (ScriptObject*)nullptr;
		return;
	}
	m_errorText.Empty();
	m_parseError->Clear();

	CArray<const char*, const char*> params;
	for (auto pair = m_parameters.PGetFirstAssoc(); pair != nullptr; pair = m_parameters.PGetNextAssoc(pair))
	{
		params.Add(pair->key.GetString());
		params.Add(pair->value.GetString());
	}
	params.Add(nullptr);

	xsltTransformContextPtr transformCtx = xsltNewTransformContext(m_stylesheet, docRef->GetDoc());
	xsltSetTransformErrorFunc(transformCtx, this, XsltTransformError);
	xmlDocPtr docResult = xsltApplyStylesheetUser(m_stylesheet, docRef->GetDoc(), params.GetData(), nullptr, nullptr, transformCtx);
	xsltFreeTransformContext(transformCtx);

	if (docResult)
		ctx.pReturnValue->Attach(new VbXmlDocument(docResult));
	else
	{
		*ctx.pReturnValue = (ScriptObject*)nullptr;
		m_parseError->Set(-1, m_errorText);
	}
}

VB_DECLARE_TYPE(VbXsltProcessor)
VB_DECLARE_CONSTRUCTOR(VbXsltProcessor, vbsNewObject)
VB_DECLARE_GETPROPERTY(VbXsltProcessor, vbsGetParseError, "ParseError")
VB_DECLARE_FUNCTION(VbXsltProcessor, vbsLoadXml, "LoadXml", 1)
VB_DECLARE_FUNCTION(VbXsltProcessor, vbsSetParameter, "SetParameter", 2)
VB_DECLARE_FUNCTION(VbXsltProcessor, vbsTransform, "Transform", 1)

}}
