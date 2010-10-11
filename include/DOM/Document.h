#ifndef __DOCUMENT_H__
#define __DOCUMENT_H__

#include <iostream>
#include <list>

#include "DOM/DOMCommonInc.h"
#include "DOM/Node.h"
#include "DOM/DocumentType.h"
#include "DOM/DOMImplementation.h"
#include "DOM/Element.h"

namespace DOM
{
  class Document : public Node
  {
  protected:  
    // DOM spec
    const DocumentTypePtr       _doctype;
    const DOMImplementationPtr  _implementation;
    
    // impl need
    std::map<DOMString, DOMString>  _prefixedNamespaces;
    std::list<DOMString>            _unprefixedNamepspaces;
    bool                        _attributeDefaultQualified;
    bool                        _elementDefaultQualified;

    ElementP                  _currentElement;
    bool                        _stateful;
    bool                        _buildingFromInputStream;
    bool                        _prettyPrint;

    // prolog properties, logically NOT part of Document
    XmlDecl                     _xmlDecl;

  public:

    Document(DocumentType*       doctype=NULL,
             DOMImplementation*  implementation=NULL
            );

    virtual ~Document() {}

    inline virtual const DocumentType* getDocType() const {
      return _doctype;
    }

    inline virtual const DOMImplementation* getImplementation() const {
      return _implementation;
    }


    virtual ElementP createElement(DOMString* tagName); // throws();
    virtual DocumentFragment* createDocumentFragment();
    virtual TextNodeP createTextNode(DOMString* data);
    virtual Comment* createComment(DOMString* data);
    virtual CDATASection* createCDATASection(DOMString* data); //  throws();
    virtual PI* createProcessingInstruction(DOMString* target,
                                                      DOMString* data); // throws();
    virtual AttributeP createAttribute(DOMString* name); //  throws();
    virtual EntityRef* createEntityReference(DOMString* name); //  throws();
    virtual NodeList* getElementsByTagName(DOMString* tagname);

        // Introduced in DOM Level 2:
    virtual Node* importNode(Node* importedNode, bool deep); // throws();
    virtual ElementP createElementNS(DOMString* namespaceURI,
                                       DOMString* qualifiedName); //  throws();

    virtual AttributeP createAttributeNS(DOMString* namespaceURI,
                                      DOMString* qualifiedName); 
    virtual AttributeP createAttributeNS(DOMString* nsUri, DOMString* nsPrefix,
      DOMString* localName, DOMString* value);
    virtual NodeList* getElementsByTagNameNS(DOMString* namespaceURI,
                                             DOMString* localName);
    virtual ElementP getElementById(DOMString* elementId);
    virtual ElementP getDocumentElement() ;
    virtual const Element* getDocumentElement() const;

    //impl apis
    virtual void startDocument();
    virtual void endDocument();
    virtual ElementP createElementNS(DOMString* nsURI, DOMString* nsPrefix, DOMString* localName);
    virtual void endElementNS(DOMString* nsURI, DOMString* nsPrefix, DOMString* localName);
    virtual inline void attributeDefaultQualified(bool b) {
      _attributeDefaultQualified = b;
    }
    virtual inline bool attributeDefaultQualified() const {
      return _attributeDefaultQualified;
    }
    virtual inline void elementDefaultQualified(bool b) {
      _elementDefaultQualified = b;
    }
    virtual inline bool elementDefaultQualified() const {
      return _elementDefaultQualified;
    }

    virtual inline void setCurrentElement(ElementP currentElem) {
      _currentElement = currentElem;
    }
    virtual inline ElementP getCurrentElement() {
      return _currentElement;
    }
    virtual inline void stateful(bool b) {
      _stateful = b;
    }
    virtual inline bool stateful() {
      return _stateful;
    }
    
    inline bool buildingFromInputStream() {
      return _buildingFromInputStream;
    }
  
    inline void prettyPrint(bool b) {
      _prettyPrint = b;
    }
    inline bool prettyPrint() const {
      return _prettyPrint;
    }

    bool isPrefixTaken(DOMString nsPrefixStr) const;
    void addPrefixedNamespace(DOMString nsPrefixStr, DOMString nsUriStr);
    void addUnprefixedNamespace(DOMString nsUriStr); 
    void registerNsPrefixNsUri(DOMString* nsPrefix, DOMString* nsUri);
    DOMString getImplicitNsPrefixForNsUri(DOMString nsUriStr) const;
    DOMString* getDocumentElementNsUri() ;
    const DOMString* getDocumentElementNsUri() const;
    inline const map<DOMString, DOMString>& getPrefixedNamespaces() const {
      return _prefixedNamespaces; 
    }
    inline const list<DOMString>& getUnprefixedNamespaces() const 
    {
      return _unprefixedNamepspaces; 
    }
    
    //FIXME: make this pvt fn later, and make DOMParser as friend
    void setXmlDecl(XmlDecl xmlDecl) {
      _xmlDecl = xmlDecl;
    }
    const DOMString* encoding() const {
      return _xmlDecl.encoding;
    }
    const DOMString* version() const {
      return _xmlDecl.version;
    }



  };

}

#endif
