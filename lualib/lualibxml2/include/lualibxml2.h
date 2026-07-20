#pragma once
#include <stdlib.h>
#undef near
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include "LuaBridge.h"
#include "RefCountedObject.h"
#include "RefCountedPtr.h"
#include "IteratorInterface.h"
//#include "Dump.h"
#include <string>
#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#include <libxml/xpath.h>
#include <libxml/parserInternals.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlschemas.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/xsltUtils.h>
#include <libxslt/transform.h>
#include "../hildim_force_debug_lua_state.h"

namespace luabridge {
    struct AttributeIdx;
    struct NodesIdx;

    class domDocument;
    class domNodeList;
    class domNode;
    class domXsltProcessor;
    class domXmlSchema;
    class domParseError;
    class NodeListIterator;

    typedef RefCountedObjectPtr<domDocument> RCDocument;
    typedef RefCountedObjectPtr<domNode> RCNode;
    typedef RefCountedObjectPtr<domNodeList> RCNodeList;
    typedef RefCountedObjectPtr<domXsltProcessor> RCProcessor;
    typedef RefCountedObjectPtr<domParseError> RCError;
    typedef RefCountedObjectPtr<domXmlSchema> RCSchema;
    typedef IteratorInterface<int, RCNode> IIF_Int_RCNode;
    typedef RefCountedObjectPtr<IIF_Int_RCNode> RC_IIF_Int_RCNode;
    LClosure dd;
    class domNode : public RefCountedObject
    {
    public:
        domNode(domDocument* docRef, xmlNodePtr node, bool owner);
        ~domNode(

        );
        xmlNodePtr GetNode() const { return m_node; }
        void Invalidate(bool removeRef = false);
        void AssertValid(lua_State* L, const char* funcname) const;
        domDocument* GetDocRef() { return m_docRef; }
        xmlNodePtr InsertNode(domNode* nodeRef, lua_State* L, domNode* nodeBeforeRef = nullptr);
        void UnlinkNode() noexcept;
        //
        int GetAttributeCount(lua_State* L) const;
        domNode* GetAttribute(int index, lua_State* L);
        domNode* GetAttribute(const char* name, lua_State* L);

        int GetChildCount(lua_State* L) const;
        void FillChildNodes(std::vector<domNode*> &cnilds) const;
        domNode* GetChild(int index, lua_State* L);

        int luaGetNodeType(lua_State* L) const;
        std::string luaGetBaseName(lua_State* L);
        std::string luaGetNodeName(lua_State* L) const;
        std::string luaGetName(lua_State* L) const;
        std::string luaGetPrefix(lua_State* L) const;
        std::string luaGetNamespaceURI(lua_State* L) const;
        RCDocument luaGetOwnerDocument(lua_State* L);
        RCNode luaGetParentNode(lua_State* L) const;
        RCNode luaGetFirstChild(lua_State* L) const;
        RCNode luaGetLastChild(lua_State* L) const;
        RCNode luaGetNextSibling(lua_State* L)const;
        RCNode luaGetPreviousSibling(lua_State* L) const;
        RCNodeList luaGetAttributes(lua_State* L) ;
        RCNodeList luaGetChildNodes(lua_State* L);
        std::string luaGetText(lua_State* L) const;
        void luaSetText(const char* text, lua_State* L);
        std::string  domNode::luaGetNodeValue(lua_State* L) const;
        inline std::string luaGetXml(lua_State* L) { return GetXml(L, false); }
        inline std::string luaTostring(lua_State* L) { return GetXml(L, true); }
        std::string GetXml(lua_State* L, bool format);
        void CheckXpathResult(xmlXPathObjectPtr xpathResult, xmlXPathContextPtr xpathCtx, lua_State* L) const;

        RCNode luaAppendChild(domNode* node, lua_State* L) ;
        RCNode luaCloneNode(bool deep, lua_State* L);
        RCDocument luaCloneDocument(bool deep, lua_State* L);
        bool luaHasChildNodes(lua_State* L) const;
        RCNode luaInsertBefore(domNode* node, domNode* nodeBefore, lua_State* L);
        void luaRemoveChild(domNode* nodeRef, lua_State* L);
        RCNode luaSelectSingleNode(const char* xpath, lua_State* L) const;
        RCNodeList luaGetElementsByTagName(const char* name, lua_State* L);
        RCNode luaGetElementByTagName(const char* name, bool deep, lua_State* L);
        RCNodeList luaSelectNodes(const char* name, lua_State* L);
        std::string luaCompareDocumentPosition(domNode* node2Compare, lua_State* L);
        std::string luaGetAttribute(std::string name, lua_State* L) ;
        void luaSetAttribute(std::string name, std::string value, lua_State* L);

        void luaRemoveAttribute(const char* name, lua_State* L);
        void SetDocRef(domDocument* docRef);
        
        
        inline bool hasChildren() const { return m_node->children != nullptr; }
        inline bool hasNextSibling() const { return m_node->next != nullptr; }

    protected:
        friend class domlDocument;
        domNode() {}  // needed for VbXmlDocument constructor
        void InvalidateNodeList(xmlNodePtr cur);
        void ClearOwnership() { m_isNodeOwner = false; }
        void findElementsByName(const std::string& tagName, domNodeList* result);
        domNode* findElementByName(const std::string& tagName, bool deep);

    protected:
        xmlNodePtr m_node{ nullptr };
        domDocument* m_docRef{ nullptr };  // weak reference (ref count is not affected)
        bool m_isNodeOwner{ false };         // true for 'dangling' nodes only (without parent)
    };

    class domDocument : public domNode
    {
    public:

        domDocument();
        domDocument(xmlDocPtr doc);
        ~domDocument(

        );
        xmlDocPtr GetDoc() const { return m_doc; }

        domNode* CreateNodeRef(xmlNodePtr node, bool setIncr = false);
        domNode* GetNodeRef(xmlNodePtr node);
        void RemoveNodeRef(domNode* nodeRef);
        domNode* RemoveNodeRef(xmlNodePtr node);
        void AttachNodeRef(domNode* nodeRef);
        //
        xmlXPathContextPtr NewXPathContext(lua_State* L);
        //
        //       static void domNewObject(vb::CallContext& ctx);
        //
        RCNode luaGetDocumentElement();

        RCError luaGetParseError() const;
        bool m_preserveSpaces{ false };

        void luaSetValidator(domXmlSchema* schema);
        bool luaLoadXml(const char* xml);

        RCNode luaCreateURINode(int type, const std::string name, const std::string namespaceURI, lua_State* L);
        inline RCNode luaCreateNode(int type, const std::string name, lua_State* L) { return luaCreateURINode(type, name, "", L); }
        RCNode luaCreateElement(const char* name);
        std::string luaGetProperty(const std::string name, lua_State* L);
        void luaSetProperty(const std::string name, std::string value, lua_State* L);

    private:
        void FreeDoc();

        void AttachNodeRef(domDocument* sourceDocRef, xmlNodePtr cur, bool sameForSiblings);
        xmlNsPtr AllocTempNamespace(const char* prefix, const char* uri);

    private:
        xmlDocPtr m_doc;
        domParseError* m_parseError;
        domXmlSchema* m_xmlScema = nullptr;
        std::map<xmlNodePtr, domNode*> m_mapNodeRefs;  // alive ScriptObjects created for VB
        std::map<std::string, std::string> m_selectNamespaces;
        std::string m_propNamespaces;
        xmlNsPtr m_nsList{ nullptr };
    };

    class domParseError : public RefCountedObject
    {
    public:
        int m_errorCode{ 0 }, m_line{ 0 }, m_linePos{ 0 };
        std::string m_reasonText;

        domParseError() { incReferenceCount(); }
        domParseError(domParseError* p) :
            m_errorCode(p->m_errorCode),
            m_line(p->m_line),
            m_linePos(p->m_linePos),
            m_reasonText(p->m_reasonText) {}

        void Set(const xmlError* error);
        void Set(int errorCode, const std::string& text);
        void Clear();

    };


    class domNodeList : public RefCountedObject//, public VbEnumerable
    {
    public:

        RC_IIF_Int_RCNode luaGetIterator();
        domNodeList(){}
        domNodeList(domNode* node, bool attributes);
        domNodeList(domDocument* doc, xmlNodePtr* nodeArray, int count);
        virtual ~domNodeList();

         domNode* GetItem(LUA_INTEGER i, lua_State* L);

        inline bool IsAttributeList()  const{ return m_isAttributeList; }
        void AddXmlNode(domDocument* doc, xmlNodePtr node);

        size_t luaGetLength(lua_State* L);
        RCNode luaGetItem(int index, lua_State* L);

        RCNode luaGetNamedItem(const char* name, lua_State* L); // available when m_isAttributeList is true
        RCNode luaSetNamedItem(domNode* node, lua_State* L);  // available when m_isAttributeList is true
    private:
        bool m_isAttributeList{ false };
        int m_index{ 0 };
        std::vector<domNode*> m_nodes ;
        domNode* m_nodeRef{ nullptr };
    };

    class NodeListIterator : public IIF_Int_RCNode
    {
    private:
        domNodeList* m_parent;
        ~NodeListIterator() {
            if (m_parent)  this->
                m_parent->decReferenceCount();
            m_parent = nullptr;
        };
        int count;
    public:
        NodeListIterator(domNodeList* parent)

        {
            m_parent = parent;
            m_parent->incReferenceCount();
            value = nullptr;
            key = -1;
            count = m_parent->luaGetLength(nullptr);
            done = (count <= 0);
        }

        void next() override
        {
            if (++key < count) {               
                value = RCNode(m_parent->GetItem(key, nullptr));
                return;
            }
            done = true;
            key = 0;
            value = 0;
            m_parent->decReferenceCount();
            m_parent = nullptr;
        }

    };

    class domXmlSchema : public RefCountedObject
    {
    public:

        domXmlSchema();
        ~domXmlSchema();

        static RCSchema luaNewObject();

        RCError luaGetParseError() const;
        bool luaLoadXml(const char* xml);
        inline bool SchemaLoaded() { return m_schema != nullptr; }
        bool luaValidate(domDocument* docRef);
        int InrernalValidate(xmlDocPtr doc);
        std::string luaTostring(lua_State* L);

    private:
        void FreeSchema();

    private:
        xmlSchemaPtr m_schema;
        xmlDocPtr m_schemaDoc;
        domParseError* m_parseError;
    };

    class domXsltProcessor : public RefCountedObject
    {
    public:
        domXsltProcessor();
        ~domXsltProcessor();

        void AddErrorText(const std::string& errorText);

        static RCProcessor luaNewObject();

        RCError luaGetParseError() const;

        void luaSetParameter(const char* name, const char* value);
        std::string luaGetTransformLog() const;

        bool luaLoadXml(const char* xml);
        RCDocument luaTransform(domDocument* docRef);

    private:
        void FreeStylesheet();

    private:
        xsltStylesheetPtr m_stylesheet;
        domParseError* m_parseError;
        std::string m_errorText;
        std::map<std::string, std::string> m_parameters;
    };
}
