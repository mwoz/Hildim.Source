// luahunspell.cpp : Defines the exported functions for the DLL application.
//
#include<lualibxml2.h>
// Use as a param to function (does not outlive a function call)
#define XML2CHR(s) reinterpret_cast<const char*>(s)

// Use as a param to function (does not outlive a function call)
#define CHR2XML(s) reinterpret_cast<const xmlChar*>(s)
static lua_State* GL;

namespace luabridge {
    static LuaRef print = NULL;
    static LuaRef tracebak = NULL;
    static LuaRef error = NULL;
    static inline void luaError(const char* txt) {
        error(txt);
    }

    static void XMLCDECL XmlCommonErrorHandler(void* ctx, const char* msg, ...)
    {
        // переделать для показа диагностики через принт?

#if defined(XML2_DEBUG)
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

    static void BuildNodeText(xmlNodePtr cur, std::string& result, size_t& firstCdataStart, size_t& lastCdataEnd)
    {
        switch (cur->type)
        {
        case XML_CDATA_SECTION_NODE:
            if (firstCdataStart < 0)
                firstCdataStart = result.length();
            result.append(XML2CHR(cur->content));
            lastCdataEnd = result.length();
            break;

        case XML_TEXT_NODE:
            result.append(XML2CHR(cur->content));
            break;

        case XML_DOCUMENT_NODE:
        case XML_DOCUMENT_FRAG_NODE:
        case XML_ELEMENT_NODE:
        case XML_ATTRIBUTE_NODE:
        {
            xmlNodePtr tmp = cur->children;
            while (tmp != NULL)
            {
                BuildNodeText(tmp, result, firstCdataStart, lastCdataEnd);
                tmp = tmp->next;
            }
            break;
        }

        case XML_ENTITY_REF_NODE:
        {
            xmlEntityPtr ent = xmlGetDocEntity(cur->doc, cur->name);
            if (ent == NULL)
                return;

            xmlNodePtr tmp = ent->children;
            while (tmp)
            {
                BuildNodeText(tmp, result, firstCdataStart, lastCdataEnd);
                tmp = tmp->next;
            }
            break;
        }
        }
    }

    domNode::domNode(domDocument* docRef, xmlNodePtr node, bool owner)
    {
        m_docRef = docRef;
        m_node = node;
        m_isNodeOwner = owner;

        if (m_docRef != dynamic_cast<domNode*>(this))
            m_docRef->incReferenceCount();  // hold a reference to the document
    }
    domNode::~domNode()
    {
        Invalidate(true);
    }
    void  domNode::Invalidate(bool removeRef)
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
        if (m_docRef && dynamic_cast<domNode*>(m_docRef) != this)
            m_docRef->decReferenceCount();
        m_docRef = nullptr;
        m_node = nullptr;
    }
    void domNode::AssertValid(lua_State* L, const char* funcname) const
    {
        if (m_node == nullptr)
            LuaException::Throw(LuaException(L, funcname, "Invalid XML node.", 1));
    }

    xmlNodePtr domNode::InsertNode(domNode* nodeRef, lua_State* L, domNode* nodeBeforeRef ) {
        if (nodeBeforeRef && nodeBeforeRef->GetNode()->parent != m_node)
            LuaException::Throw(LuaException(L, "InsertNode", "'Before' node is not a child node.", 1));

        nodeRef->UnlinkNode();

        bool reconcileNs = false;
        if (GetDocRef() != nodeRef->GetDocRef())
        {
            auto wrapCtx = xmlDOMWrapNewCtxt();
            int wrapResult = xmlDOMWrapAdoptNode(wrapCtx, nodeRef->GetDocRef()->GetDoc(), nodeRef->GetNode(), m_docRef->GetDoc(), m_node, 0);
            xmlDOMWrapFreeCtxt(wrapCtx);
            if (wrapResult != 0)
                LuaException::Throw(LuaException(L, "InsertNode", "Invalid XML node for adoption into another document.", 1));
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
            LuaException::Throw(LuaException(L, "InsertNode", "Failed to add child node.", 1));

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
    void domNode::UnlinkNode() noexcept {
        xmlUnlinkNode(m_node);
        m_isNodeOwner = true;
     }
    
    int domNode::GetAttributeCount(lua_State* L) {
        AssertValid(L, "GetAttributeCount");

        if (m_node->type != XML_ELEMENT_NODE)
            return 0;

        int count = 0;
        for (auto n = m_node->properties; n != nullptr; n = n->next)
            count++;

        return count;
    }
    domNode* domNode::GetAttribute(int index, lua_State* L)
    {
        AssertValid(L, "GetAttribute by index");

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

        return m_docRef->CreateNodeRef(node);
    }
    domNode* domNode::GetAttribute(const char* name, lua_State* L)
    {
        AssertValid(L, "GetAttribute by index");

        if (m_node->type != XML_ELEMENT_NODE)
            LuaException::Throw(LuaException(L, "GetAttribute by index", "Invalid XML node.", 1));

        return m_docRef->CreateNodeRef((xmlNodePtr)xmlHasProp(m_node, CHR2XML(name)));
    }
    int domNode::GetChildCount(lua_State* L) {
        AssertValid(L, "GetChildCount");

        int count = 0;
        for (auto n = m_node->children; n != nullptr; n = n->next)
            count++;

        return count;
    }
    domNode* domNode::GetChild(int index, lua_State* L) {
        AssertValid(L, "GetChild");

        int i = 0;
        xmlNodePtr n;
        for (n = m_node->children; i < index && n != nullptr; n = n->next)
            i++;

        xmlNodePtr node = nullptr;
        if (i == index)
            node = (xmlNodePtr)n;

        return (m_docRef->CreateNodeRef(node));
    }


    int domNode::luaGetNodeType(lua_State* L)
    {
        AssertValid(L, "luaGetNodeType");
        return m_node->type;
    }

    std::string domNode::luaGetBaseName(lua_State* L)
    {
        AssertValid(L, "luaGetBaseName");
        return XML2CHR(m_node->name);
    }
    std::string domNode::luaGetNodeName(lua_State* L)
    {
        AssertValid(L, "luaGetNodeName");
        std::string result;
 
        if ((m_node->type == XML_ATTRIBUTE_NODE || m_node->type == XML_ELEMENT_NODE) && m_node->ns) {
            result = XML2CHR(m_node->ns->prefix);
            result += ':';
            result += XML2CHR(m_node->name);
        }
        else
            result = XML2CHR(m_node->name);
        return result;
    }
   std::string domNode::luaGetName(lua_State* L)
    {
        AssertValid(L, "luaGetName");
        std::string result;
 
        if (m_node->type != XML_ATTRIBUTE_NODE)
            LuaException::Throw(LuaException(L, "luaGetName", "Property 'name' is supported for XML_ATTRIBUTE_NODE only.", 1));

        if (m_node->ns) {
            result = XML2CHR(m_node->ns->prefix);
            result += ':';
            result += XML2CHR(m_node->name);
        }
        else
            result = XML2CHR(m_node->name);
        return result;
    }
   std::string domNode::luaGetPrefix(lua_State* L)
    {
        AssertValid(L, "luaGetPrefix");
        std::string result;
        if ((m_node->type == XML_ATTRIBUTE_NODE || m_node->type == XML_ELEMENT_NODE) && m_node->ns)
            result = XML2CHR(m_node->ns->prefix);
        return result;
    }
    std::string domNode::luaGetNamespaceURI(lua_State* L)
    {
        AssertValid(L, "luaGetNamespaceURI");
        std::string result;
        if ((m_node->type == XML_ATTRIBUTE_NODE || m_node->type == XML_ELEMENT_NODE) && m_node->ns)
            result = XML2CHR(m_node->ns->href);
        return result;
    }

    RCDocument domNode::luaGetOwnerDocument(lua_State* L){
        AssertValid(L, "luaGetOwnerDocument");
        if (this == m_docRef)
            return RCDocument(nullptr);
        else
            return RCDocument(m_docRef);
    }
    RCNode domNode::luaGetParentNode(lua_State* L){
        AssertValid(L, "luaGetParentNode");
        return RCNode(m_docRef->CreateNodeRef(m_node->parent));
    }
   RCNode domNode::luaGetFirstChild(lua_State* L){
        AssertValid(L, "luaGetFirstChild");
        return RCNode(m_docRef->CreateNodeRef(m_node->children));
    }
   RCNode domNode::luaGetLastChild(lua_State* L){
        AssertValid(L, "luaGetLastChild");
        return RCNode(m_docRef->CreateNodeRef(m_node->last));
    }
   RCNode domNode::luaGetNextSibling(lua_State* L){
        AssertValid(L, "luaGetNextSibling");
        return RCNode(m_docRef->CreateNodeRef(m_node->next));
    }
   RCNode domNode::luaGetPreviousSibling(lua_State* L){
        AssertValid(L, "luaGetPreviousSibling");
        return RCNode(m_docRef->CreateNodeRef(m_node->prev));
    }
   RCNodeList domNode::luaGetAttributes(lua_State* L) {
       AssertValid(L, "luaGetAttributes");
       if (m_node->type != XML_ELEMENT_NODE)
           LuaException::Throw(LuaException(L, "luaGetAttributes", "XML node is not an element.", 1));

       return RCNodeList(new domNodeList(this, true));

   }

   RCNodeList domNode::luaGetChildNodes(lua_State* L) {
       AssertValid(L, "luaGetChildNodes");
       if (m_node->type != XML_ELEMENT_NODE)
           LuaException::Throw(LuaException(L, "luaGetChildNodes", "XML node is not an element.", 1));

       return RCNodeList(new domNodeList(this, false));
   }

   std::string domNode::luaGetText(lua_State* L) const{
       AssertValid(L, "luaGetText");

       std::string result;
       size_t startCdata = -1, endCdata = -1;
       BuildNodeText(m_node, result, startCdata, endCdata);

       // trim right till the last CDATA
       while (endCdata < result.length() && result.length() > 0)
       {
           char c = result[result.length() - 1];
           if (c == ' ' || c == '\t' || (c >= 10 && c <= 13))
               result.erase(result.cend());
           else
               break;
       }

       // trim left to the first CDATA
       if (startCdata < 0)
           startCdata = result.length();

       int spaceCount = 0;
       while (spaceCount < startCdata)
       {
           char c = result[spaceCount];
           if (c == ' ' || c == '\t' || (c >= 10 && c <= 13))
               spaceCount++;
           else
               break;
       }
       if (spaceCount > 0)
           result.erase(0, spaceCount);

       return result;
   }
   void domNode::luaSetText(const char* text, lua_State* L) {
       AssertValid(L, "luaSetText");

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

           xmlNodePtr nodeText = xmlNewDocText(m_docRef->GetDoc(), CHR2XML(text));
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
           m_node->content = xmlStrdup(CHR2XML(text));
           m_node->properties = NULL;
           break;

       default:
           LuaException::Throw(LuaException(L, "luaSetText", "Setting Text is not supported for this type of node.", 1));
       }
   }
   std::string domNode::luaGetNodeValue(lua_State* L)
   {
       AssertValid(L, "luaGetNodeValue");

       std::string result;
       switch (m_node->type)
       {
       case XML_ATTRIBUTE_NODE:
           if (m_node->children)
           {
               if (m_node->children->next == nullptr && m_node->children->type == XML_TEXT_NODE)  // optimization for the common case
                   result = XML2CHR(m_node->children->content);
               else
               {
                   auto tmp = xmlNodeGetContent(m_node);
                   result = XML2CHR(tmp);
                   xmlFree(tmp);
               }
           }
           break;

       case XML_CDATA_SECTION_NODE:
       case XML_TEXT_NODE:
       case XML_COMMENT_NODE:
           result = XML2CHR(m_node->content);
           break;

       default:
           break;
       }

       return result;
   }
    std::string domNode::GetXml(lua_State* L, bool format)
    {
        AssertValid(L, "luaGetXml");

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
        xmlSaveCtxtPtr saveCtx = xmlSaveToBuffer(buf, "UTF-8", XML_SAVE_AS_XML | XML_SAVE_NO_DECL | (format ? XML_SAVE_FORMAT : 0) );
        auto resultCode = xmlSaveTree(saveCtx, m_node);
        if (resultCode < 0)
        {
            xmlSaveClose(saveCtx);
            m_node->nsDef = nsDef_Saved;  // restore ns declaration

            LuaException::Throw(LuaException(L, "luaGetXml", "Failed to dump XML.", 1));
        }
        xmlSaveFlush(saveCtx);

        //xmlNodeDump(buf, m_docRef->GetDoc(), m_node, 0, 0);
        std::string result = XML2CHR(xmlBufferContent(buf));// XmlCharsToString(xmlBufferContent(buf), xmlBufferLength(buf));
        xmlSaveClose(saveCtx);
        m_node->nsDef = nsDef_Saved;  // restore ns declaration

       return result.c_str();
    }

    RCNode domNode::luaAppendChild(domNode* node, lua_State* L) {
        AssertValid(L, "luaAppendChild");

       if (node->GetNode()->type == XML_ATTRIBUTE_NODE)
           LuaException::Throw(LuaException(L, "luaAppendChild", "Attribute is not allowed in this method.", 1));

        return RCNode(m_docRef->CreateNodeRef(InsertNode(node, L)));
    }
    RCNode domNode::luaCloneNode(bool deep, lua_State* L) {
        AssertValid(L, "luaCloneNode");
        
        xmlNodePtr node = xmlDocCopyNode(m_node, m_docRef->GetDoc(), deep ? 1 : 0);
        if (node->type == XML_DOCUMENT_NODE)
            LuaException::Throw(LuaException(L, "luaCloneNode", "XmlDocument is not allowed in this method. Use cloneDocument", 1));

        return RCNode(m_docRef->CreateNodeRef(node));
    }
    RCDocument domNode::luaCloneDocument(bool deep, lua_State* L) {
        AssertValid(L, "luaCloneDocument");

        xmlNodePtr node = xmlDocCopyNode(m_docRef->GetNode(), m_docRef->GetDoc(), deep ? 1 : 0);
        return RCDocument(new domDocument((xmlDocPtr)node));

    }
    bool domNode::luaHasChildNodes(lua_State* L)
    {
        AssertValid(L, "luaHasChildNodes");
        return (bool)(m_node->children != nullptr);
    }

    RCNode domNode::luaInsertBefore(domNode* node, domNode* nodeBefore, lua_State* L)
    {
        AssertValid(L, "luaInsertBefore");

        if (node->GetNode()->type == XML_ATTRIBUTE_NODE)
            LuaException::Throw(LuaException(L, "luaCloneNode", "Attribute is not allowed in this method", 1));
        return RCNode(m_docRef->CreateNodeRef(InsertNode(node, L, nodeBefore)));
    }

    void domNode::luaRemoveChild(domNode* nodeRef, lua_State* L)
    {
        AssertValid(L, "luaRemoveChild");

        if (nodeRef->GetNode()->parent != m_node)
            LuaException::Throw(LuaException(L, "luaAppendChild", "XML node is not the child node.", 1));

        nodeRef->UnlinkNode();
    }
    RCNode domNode::luaSelectSingleNode(const char* xpath, lua_State* L)
    {
        AssertValid(L, "luaSelectSingleNode");
        RCNode result;
        auto xpathCtx = m_docRef->NewXPathContext(L);
        if (xmlXPathSetContextNode(m_node, xpathCtx) != 0)
        {
            xmlXPathFreeContext(xpathCtx);
            LuaException::Throw(LuaException(L, "luaSelectSingleNode", "Invalid node for XPath search.", 1));
        }

        auto xpathResult = xmlXPathEvalExpression(CHR2XML(xpath), xpathCtx);
        xmlXPathFreeContext(xpathCtx);
        if (xpathResult == NULL)
        {
            std::string s = "Invalid XPath expression ";
            s += xpath;
            s += ".";
            LuaException::Throw(LuaException(L, "luaSelectSingleNode", s.c_str(), 1));
        }

        if (xpathResult->nodesetval == nullptr || xpathResult->nodesetval->nodeNr == 0)
            return RCNode(nullptr);
        else
            result = RCNode(m_docRef->CreateNodeRef(xpathResult->nodesetval->nodeTab[0]));
        xmlXPathFreeObject(xpathResult);
        return result;
    }

    RCNodeList domNode::luaSelectNodes(const char* xpath, lua_State* L) {
        AssertValid(L, "luaSelectNodes");
        RCNodeList result;
        auto xpathCtx = m_docRef->NewXPathContext(L);
        if (xmlXPathSetContextNode(m_node, xpathCtx) != 0)
        {
            xmlXPathFreeContext(xpathCtx);
            LuaException::Throw(LuaException(L, "luaSelectNodes", "Invalid node for XPath search.", 1));
        }

        auto xpathResult = xmlXPathEvalExpression(CHR2XML(xpath), xpathCtx);
        xmlXPathFreeContext(xpathCtx);
        if (xpathResult == NULL)
        {
            std::string s = "Invalid XPath expression ";
            s += xpath;
            s += ".";
            LuaException::Throw(LuaException(L, "luaSelectNodes", s.c_str(), 1));
        }
        auto noteSet = xpathResult->nodesetval;
        domNodeList* l = new domNodeList(m_docRef, noteSet ? noteSet->nodeTab : nullptr, noteSet ? noteSet->nodeNr : 0);
       // l->incReferenceCount()
        result = RCNodeList(l);
        xmlXPathFreeObject(xpathResult);
        return result;

    }
//        void SelectSingleNode(vb::CallContext& ctx);
    std::string domNode::luaGetAttribute(const char* name, lua_State* L) {
        AssertValid(L, "luaGetAttribute");


        if (m_node->type != XML_ELEMENT_NODE) {
            LuaException::Throw(LuaException(L, "luaGetAttribute", "XML node is not an element.", 1));
        }

        xmlAttrPtr prop = xmlHasProp(m_node, CHR2XML(name));
        if (prop)
        {
            std::string result;
            if (prop->children->next == nullptr && prop->children->type == XML_TEXT_NODE)  // optimization for the common case
                result = XML2CHR(prop->children->content);
            else
            {
                auto tmp = xmlNodeGetContent((xmlNodePtr)prop);
                result = XML2CHR(tmp);
                xmlFree(tmp);
            }
            return result;
        }
        return "";
    }
    void domNode::luaSetAttribute(const char* name, const char* value, lua_State* L)
    {
        AssertValid(L, "luaSetAttribute");

        if (m_node->type != XML_ELEMENT_NODE) {
            LuaException::Throw(LuaException(L,"luaSetAttribute", "XML node is not an element.", 1));
        }

        xmlSetProp(m_node, CHR2XML(name), CHR2XML(value));
    }

    RefCountedObjectPtr<AttributeIdx> domNode::luaAttribute() {
        if(!m_pAttributeIdx)
            m_pAttributeIdx = new AttributeIdx(this);

        return RefCountedObjectPtr<AttributeIdx>(m_pAttributeIdx);

    }
    void domNode::luaRemoveAttribute(const char* name, lua_State* L)
    {
        AssertValid(L, "luaRemoveAttribute");

        if (m_node->type != XML_ELEMENT_NODE)
            LuaException::Throw(LuaException(L, "luaSetAttribute", "XML node is not an element.", 1));

        xmlAttrPtr prop = xmlHasProp(m_node, CHR2XML(name));
        if (prop == nullptr)
            return;

        auto nodeRef = m_docRef->GetNodeRef((xmlNodePtr)prop);
        if (nodeRef)
            nodeRef->UnlinkNode();
        else
            xmlRemoveProp(prop);
    }

   void domNode::SetDocRef(domDocument* docRef) {
        //ASSERT(m_docRef != this);
        //ASSERT(docRef != this);

        if (m_docRef)
            m_docRef->decReferenceCount();
        m_docRef = docRef;
        m_docRef->incReferenceCount();
    }

    void domNode::InvalidateNodeList(xmlNodePtr cur) {

    }



    domDocument::domDocument()
        {
            m_parseError = new domParseError();
            m_docRef = this;
            m_doc = xmlNewDoc(nullptr);
            m_node = (xmlNodePtr)m_doc;
        }
    domDocument::domDocument(xmlDocPtr doc)
        {
            m_parseError = new domParseError();
            m_docRef = this;
            m_doc = doc;
            m_node = (xmlNodePtr)doc;
        }
    domDocument::~domDocument() {
        m_parseError->decReferenceCount();
        m_parseError = nullptr;
        FreeDoc();
        m_docRef = nullptr;
    }

    domNode* domDocument::CreateNodeRef(xmlNodePtr node) {
        if (node == nullptr)
            return nullptr;

        assert(m_doc == node->doc);
        if (m_doc != node->doc)
            return nullptr;
 
        if (node == m_node)
        {
            incReferenceCount();
            return this;
        }
 
        domNode* result;
        auto pair = m_mapNodeRefs.find(node);
        if (pair != m_mapNodeRefs.cend())
        {
            result = m_mapNodeRefs.at(node);
            result->incReferenceCount();
        }
        else
        {
            result = new domNode(this, node, node->type != XML_DOCUMENT_NODE && node->parent == nullptr);
            result->incReferenceCount();
            m_mapNodeRefs[node] = result;// .SetAt(node, result);
        }
        return result;
    }

   
    domNode* domDocument::GetNodeRef(xmlNodePtr node) {
       // auto pair = m_mapNodeRefs.at(node);
       // if (pair)
      //      return pair->value; 

        return m_mapNodeRefs.at(node);
       }

    void domDocument::RemoveNodeRef(domNode* nodeRef) {
        m_mapNodeRefs.erase(nodeRef->GetNode());
    }
    domNode* domDocument::RemoveNodeRef(xmlNodePtr node) {
        auto result = m_mapNodeRefs.find(node);//    PLookup(node);

        if (result == m_mapNodeRefs.cend())
            return nullptr;

        domNode* nodeRef = m_mapNodeRefs.at(node); // result->first(); //result.  // ((result.;
        m_mapNodeRefs.erase(node);  // .RemoveKey(node);
        return nodeRef;
    }
    void domDocument::AttachNodeRef(domNode* nodeRef) {
        //ASSERT(nodeRef && nodeRef->GetNode() && nodeRef->GetDocRef());

        if (nodeRef->GetDocRef() == this)
            return;

        AttachNodeRef(nodeRef->GetDocRef(), nodeRef->GetNode(), false);
      }
//
    xmlXPathContextPtr domDocument::NewXPathContext(lua_State* L)
    {
        auto xpathCtx = xmlXPathNewContext(m_docRef->GetDoc());
        if (xpathCtx == NULL)
            LuaException::Throw(LuaException(L, "NewXPathContext", "Failed to create XPath context.", 1));

        auto iterator = m_selectNamespaces.begin();
        while (iterator != m_selectNamespaces.cend())
        {
            const char* prefix = iterator->first.c_str();
            const char* uri = iterator->second.c_str();
            if (xmlXPathRegisterNs(xpathCtx, (const xmlChar*)prefix, (const xmlChar*)uri) != 0)
            {
                xmlXPathFreeContext(xpathCtx);

                std::string s = "Failed to register namespace (prefix=";
                s += prefix;
                s += ", uri=";
                s += uri;
                s += ").";
                LuaException::Throw(LuaException(L, "NewXPathContext", s.c_str(), 1));
            }
            iterator++;
        }
        return xpathCtx;
    }
//
//       static void domNewObject(vb::CallContext& ctx);
//
    RCNode domDocument::luaGetDocumentElement() {
        domNode* p = CreateNodeRef(m_doc->children);
        return RCNode(p);
    }
    
    //       void domGetParseError(vb::CallContext& ctx);
//         
//       void domGetPreserveWhiteSpace(vb::CallContext& ctx);
//       void domSetPreserveWhiteSpace(vb::CallContext& ctx);
         
    bool domDocument::luaLoadXml(const char* xml)
    {
        auto parserCtx = xmlCreateDocParserCtxt(CHR2XML(xml));
        xmlSwitchEncodingName(parserCtx, "UTF-8");

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

        return (result == 0);
    }
    RefCountedObjectPtr<domParseError> domDocument::luaGetParseError() const {
        return RefCountedObjectPtr<domParseError>(m_parseError);
    };
    RCNode domDocument::luaCreateURINode(int type, const std::string name, std::string namespaceURI, lua_State* L){
 
        // if namespace specified name can be in format 'prefix:basename'
        std::string prefix;
        std::string nameBase = name;
        xmlNsPtr ns = nullptr;
        if (type == XML_ELEMENT_NODE || type == XML_ATTRIBUTE_NODE)
        {

            size_t pos = name.find(':');
            if (pos != std::string::npos)
            {
                if (namespaceURI == "")
                    LuaException::Throw(LuaException(L, "luaCreateNode", "Invalid argument 'name': prefix specified but 'namespaceURI' is empty.", 1));

                prefix.assign(name.c_str(), pos);
                nameBase.assign(name.c_str() + pos + 1);
            }
            if (namespaceURI != "")
                ns = AllocTempNamespace(prefix.c_str(), namespaceURI.c_str());
        }
        else
            nameBase = name;

        domNode* nodeRef = nullptr;
        switch (type)
        {
        case XML_ELEMENT_NODE:
            if (!nameBase.size())
                LuaException::Throw(LuaException(L, "luaCreateNode", "Element name must not be empty.", 1));
            nodeRef = CreateNodeRef(xmlNewDocNode(m_doc, ns, CHR2XML(nameBase.c_str()), nullptr));
            break;
        case XML_ATTRIBUTE_NODE:
            if (!nameBase.size())
                LuaException::Throw(LuaException(L, "luaCreateNode", "Attribute name must not be empty.", 1));
            nodeRef = CreateNodeRef((xmlNodePtr)xmlNewDocProp(m_doc, CHR2XML(nameBase.c_str()), nullptr));
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
            LuaException::Throw(LuaException(L, "luaCreateNode", "Unsupported node type", 1));
        }
        return RCNode(nodeRef);
    }
    RCNode domDocument::luaCreateElement(const char* name)
    {
        domNode* n = CreateNodeRef(xmlNewDocNode(m_doc, nullptr, CHR2XML(name), nullptr));
        return RCNode(n);
    }
    //    void domGetProperty(vb::CallContext& ctx);       
    //    void domSetProperty(vb::CallContext& ctx);

 /*   const std::string  domDocument::luaGetProperty(const char* name, lua_State* L)
    {
        if (name == "SelectionNamespaces")
        {
            return m_propNamespaces;
        }
        else if (name == "SelectionLanguage")
        {
            return std::string("XPath");
        }
        else
            LuaException::Throw(LuaException(L, "luaGetProperty", "Unsupported property name.", 1));

    }
    // trim from start (in place)
    inline void ltrim(std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
            }));
    }
    // trim from end (in place)
    inline void rtrim(std::string& s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
            }).base(), s.end());
    }
    void domDocument::luaSetProperty(const std::string name, const std::string value, lua_State* L)
    {

        if (name == "SelectionNamespaces")
        {

            std::map<std::string, std::string> namespaces;
            std::string s = value;
            ltrim(s);
            rtrim(s);
            int xmlnsLen = static_cast<int>(strlen("xmlns:"));
            while (!s.empty())
            {
                if (s.find("xmlns:") != 0)
                    LuaException::Throw(LuaException(L, "luaSetProperty", "Invalid value for property 'SelectionNamespaces': every item should start with 'xmlns:'.", 1));

                int posSeparator = s.find('=');
                if (posSeparator == std::string::npos)
                    LuaException::Throw(LuaException(L, "luaSetProperty", "Invalid value for property 'SelectionNamespaces': no corresponding '='.", 1));

                std::string prefix = s.substr(xmlnsLen, posSeparator - xmlnsLen);
                rtrim(prefix);
                if (xmlValidateNCName(CHR2XML(prefix.c_str()), 0) != 0)
                    LuaException::Throw(LuaException(L,"luaSetProperty",  "Invalid value for property 'SelectionNamespaces': invalid prefix.", 1));

                s.erase(0, posSeparator + 1);
                ltrim(s);
                if (s.find('\'') != 0)
                    LuaException::Throw(LuaException(L, "luaSetProperty", "Invalid value for property 'SelectionNamespaces': string literal for URI not found.", 1));

                posSeparator = s.find('\'', 1);
                if (posSeparator == std::string::npos)
                    LuaException::Throw(LuaException(L, "luaSetProperty", "Invalid value for property 'SelectionNamespaces': string literal for URI not found.", 1));

                std::string uri = s.substr(1, posSeparator - 1);
                if (namespaces.PLookup(prefix))
                    LuaException::Throw(LuaException(L, "luaSetProperty", "Invalid value for property 'SelectionNamespaces': non-unique prefix.", 1));

                namespaces.insert(prefix, uri);

                s.Delete(0, posSeparator + 1);
                if (!s.IsEmpty() && !_istspace(s[0]))
                    LuaException::Throw(LuaException(L, "luaSetProperty", "Invalid value for property 'SelectionNamespaces': no whitespace between items.", 1));

                ltrim(s);
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
        else if (name == "SelectionLanguage")
        {
            CString value = GetString(*ctx.pParams[1]);
            if (value.CompareNoCase(_T("XPath")) != 0)
                throw VbRuntimeException(VbErrorNumber::InvalidArgument, _T("Unsupported value for property 'SelectionLanguage'."));
        }
        else
            LuaException::Throw(LuaException(L, "luaSetProperty", "Unsupported property name.", 1));
    }*/
    void domDocument::FreeDoc()
    {
        auto iterator = m_mapNodeRefs.begin();
        while(iterator != m_mapNodeRefs.cend()){
            m_mapNodeRefs.at(iterator->first)->Invalidate();
            iterator++;
        }

        m_mapNodeRefs.clear();

        if (m_doc)
            xmlFreeDoc(m_doc);

        xmlFreeNsList(m_nsList);
        m_nsList = nullptr;

        m_doc = nullptr;
        m_node = nullptr;
    }
    void domDocument::AttachNodeRef(domDocument* sourceDocRef, xmlNodePtr cur, bool sameForSiblings) {
        while (cur)
        {
            if (cur->children && cur->type != XML_ENTITY_REF_NODE)
                AttachNodeRef(sourceDocRef, cur->children, true);

            if ((cur->type == XML_ELEMENT_NODE || cur->type == XML_XINCLUDE_START || cur->type == XML_XINCLUDE_END) && cur->properties)
                AttachNodeRef(sourceDocRef, (xmlNodePtr)cur->properties, true);

            auto nodeRef = sourceDocRef->RemoveNodeRef(cur);
            if (nodeRef)
            {
                m_mapNodeRefs[cur] = nodeRef;
                nodeRef->SetDocRef(this);
            }

            if (!sameForSiblings)
                break;
            cur = cur->next;
        }
    }
    xmlNsPtr domDocument::AllocTempNamespace(const char* prefix, const char* uri) {
        xmlNsPtr ns = m_nsList;
        while (ns)
        {
            if (xmlStrEqual(CHR2XML(prefix), ns->prefix) && xmlStrEqual(CHR2XML(uri), ns->href))
                break;
            ns = ns->next;
        }
        if (ns)
            return ns;

        ns = xmlNewNs(nullptr, CHR2XML(uri), CHR2XML(prefix));
        ns->next = m_nsList;
        m_nsList = ns;
        return ns;
   }


    void domParseError::Set(const xmlError* error)
    {
        if (error)
        {
            m_errorCode = error->code;
            m_line = error->line;
            m_linePos = error->int2;
            m_reasonText = error->message;
        }
        else
            Clear();
    }
    void domParseError::Set(int errorCode, const std::string& text)
    {
        Clear();
        m_errorCode = errorCode;
        m_reasonText = text;
    }
    void domParseError::Clear()
    {
        m_errorCode = 0;
        m_line = -1;
        m_linePos = -1;
        m_reasonText = "";
    }

    domNodeList::domNodeList(domNode* node, bool attributes)
    {
        m_isAttributeList = attributes;
        m_nodeRef = node;
        m_nodeRef->incReferenceCount();
    }

   domNodeList::domNodeList(domDocument* doc, xmlNodePtr* nodeArray, int count)
   {
       m_isAttributeList = false; 
       for (int i = 0; i < count; i++)
           m_nodes.insert(m_nodes.end(), doc->CreateNodeRef(nodeArray[i]));

       int tt = 99;
   }
   domNodeList::~domNodeList()
   {
       for (int i = 0; i < m_nodes.size(); i++)
       {
           m_nodes[i]->decReferenceCount();
       }
       m_nodes.clear();

       if (m_nodeRef)
       {
           m_nodeRef->decReferenceCount();
           m_nodeRef = nullptr;
       }
   }
   size_t domNodeList::luaGetLength(lua_State* L)
   {
       if (m_nodeRef)
           return m_isAttributeList ? m_nodeRef->GetAttributeCount(L) : m_nodeRef->GetChildCount(L);
       else
          return m_nodes.size();
   }

   RCNode domNodeList::luaGetItem(int index, lua_State* L)
   {

       if (m_nodeRef)
       {
           if (m_isAttributeList)
               return RCNode(m_nodeRef->GetAttribute(index, L));
           else
               return RCNode(m_nodeRef->GetChild(index, L));
       }
       else
       {
           if (index < 0 || index >= m_nodes.size())
               return RCNode(nullptr);
           else
               return RCNode(m_nodes[index]);
       }
   }

   RCNode domNodeList::luaGetNamedItem(const char* name, lua_State* L)
   {
       if (!m_isAttributeList || m_nodeRef == nullptr)
           LuaException::Throw(LuaException(L, "luaGetNamedItem", "Method 'GetNamedItem' is supported in attributes collection.", 1));

       return RCNode(m_nodeRef->GetAttribute(name, L));
   }

   RCNode domNodeList::luaSetNamedItem(domNode* node, lua_State* L)
   {
       if (!m_isAttributeList || m_nodeRef == nullptr)
           LuaException::Throw(LuaException(L, "luaSetNamedItem", "Method 'SetNamedItem' is supported in attributes collection.", 1));
  
       //VbXmlNode* node = GetNodeArg(*ctx.pParams[0]);
       if (node->GetNode()->type != XML_ATTRIBUTE_NODE)
           LuaException::Throw(LuaException(L, "luaSetNamedItem", "Node parameter is not an XML attribute.", 1));
  
       if(m_nodeRef) 
           m_nodeRef->AssertValid(L, "domNodeList::luaSetNamedItem");
       return RCNode(m_nodeRef->GetDocRef()->CreateNodeRef(m_nodeRef->InsertNode(node, L)));
   }

    void bindToLUA(lua_State* L)
    {
        GL = L;
        print = getGlobal(L, "print");
        tracebak = getGlobal(L, "debug")["traceback"];
        error = getGlobal(L, "error");
        //lprint = print;
        // These all evaluate to true

        getGlobalNamespace(L)
            .beginNamespace("XML")
            .addConstant("XML_ELEMENT_NODE", 1)
            .addConstant("XML_ATTRIBUTE_NODE", 2)
            .addConstant("XML_TEXT_NODE", 3)
            .addConstant("XML_CDATA_SECTION_NODE", 4)
            .addConstant("XML_ENTITY_REF_NODE", 5)
            .addConstant("XML_ENTITY_NODE", 6)
            .addConstant("XML_PI_NODE", 7)
            .addConstant("XML_COMMENT_NODE", 8)
            .addConstant("XML_DOCUMENT_NODE", 9)
            .addConstant("XML_DOCUMENT_TYPE_NODE", 10)
            .addConstant("XML_DOCUMENT_FRAG_NODE", 11)
            .addConstant("XML_NOTATION_NODE", 12)
            .addConstant("XML_HTML_DOCUMENT_NODE", 13)
            .addConstant("XML_DTD_NODE", 14)
            .addConstant("XML_ELEMENT_DECL", 15)
            .addConstant("XML_ATTRIBUTE_DECL", 16)
            .addConstant("XML_ENTITY_DECL", 17)
            .addConstant("XML_NAMESPACE_DECL", 18)
            .addConstant("XML_XINCLUDE_START", 19)
            .addConstant("XML_XINCLUDE_END", 20)
            .beginClass <domNode>("xmlNode")
                .addProperty("xml", &domNode::luaGetXml)
                .addFunction("__tostring", &domNode::luaTostring)
                .addFunction("getAttribute", &domNode::luaGetAttribute)
                .addFunction("setAttribute", &domNode::luaSetAttribute)
                .addProperty("attribute", &domNode::luaAttribute)
                .addProperty("nodeType", &domNode::luaGetNodeType)
                .addProperty("baseName", &domNode::luaGetBaseName)
                .addProperty("nodeName", &domNode::luaGetNodeName)
                .addProperty("name", &domNode::luaGetName)
                .addProperty("prefix", &domNode::luaGetPrefix)
                .addProperty("namespaceURI", &domNode::luaGetNamespaceURI)
                .addProperty("ownerDocument", &domNode::luaGetOwnerDocument)
                .addProperty("parentNode", &domNode::luaGetParentNode)
                .addProperty("firstChildren", &domNode::luaGetFirstChild)
                .addProperty("lastChildren", &domNode::luaGetLastChild)
                .addProperty("nextSibling", &domNode::luaGetNextSibling)
                .addProperty("previousSibling", &domNode::luaGetPreviousSibling)
                .addProperty("attibutes", &domNode::luaGetAttributes)
                .addFunction("attibutes", &domNode::luaRemoveAttribute)
                .addProperty("childNodes", &domNode::luaGetChildNodes)
                .addProperty("text", &domNode::luaGetText, &domNode::luaSetText)
                .addProperty("nodeValue", &domNode::luaGetNodeValue)

                .addFunction("appendChild", &domNode::luaAppendChild)
                .addFunction("cloneNode", &domNode::luaCloneNode)
                .addFunction("cloneDocument", &domNode::luaCloneDocument) 
                .addProperty("hasChildNodes", &domNode::luaHasChildNodes)
                .addFunction("insertBefore", &domNode::luaInsertBefore)
                .addFunction("removeChild", &domNode::luaRemoveChild)
                .addFunction("selectSingleNode", &domNode::luaSelectSingleNode)
                .addFunction("selectNodes", &domNode::luaSelectNodes)
            .endClass()
            .deriveClass <domDocument, domNode>("Document")
                .addConstructor <void (*) (void), RCDocument >()
                .addFunction("__tostring", &domNode::luaTostring) 
                .addFunction("loadXml", &domDocument::luaLoadXml)
                .addFunction("ceateURINode", &domDocument::luaCreateURINode)
                .addFunction("createNode", &domDocument::luaCreateNode)
                .addFunction("createElement ", &domDocument::luaCreateElement)
                .addProperty("preserveWhiteSpace", &domDocument::m_preserveSpaces, true)
                .addProperty("parseError", &domDocument::luaGetParseError)
                .addProperty("documentElement", &domDocument::luaGetDocumentElement)
            .endClass()
            .beginClass <AttributeIdx>("AttributeIdx")
                .addFunction("__index", &AttributeIdx::get)
                .addFunction("__newindex", &AttributeIdx::set)
            .endClass()
            .beginClass <domParseError>("ParseError")
                .addProperty("errorCode", &domParseError::m_errorCode, false)
                .addProperty("line", &domParseError::m_line, false)
                .addProperty("linePos", &domParseError::m_linePos, false)
                .addProperty("reason", &domParseError::m_reasonText, false)
            .endClass()
            .beginClass <domNodeList>("NodeList")
                .addProperty("length", &domNodeList::luaGetLength)
                .addFunction("getItem", &domNodeList::luaGetItem)
                .addFunction("getNamedItem", &domNodeList::luaGetNamedItem)
                .addFunction("setNamedItem", &domNodeList::luaSetNamedItem)

            .endClass()
            .endNamespace();

    }
}
extern "C" __declspec(dllexport) int luaopen_lualibxml2(lua_State* L) {
    // copy locale from environment
    //setlocale(LC_ALL, "");

    luabridge::bindToLUA(L);

    ///lua_pushcfunction(L, g_spell);
    return 1;
}