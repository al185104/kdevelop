/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ast.h"

// ------------------------------------------------------------------------
AST::AST()
    : m_startLine( 0 ), m_startColumn( 0 ),
      m_endLine( 0 ), m_endColumn( 0 )
{
}

AST::~AST()
{
}

void AST::setStartPosition( int line, int col )
{
   m_startLine = line;
   m_startColumn = col;
}

void AST::getStartPosition( int* line, int* col ) const
{
    if( line )
        *line = m_startLine;
	
    if( col )
        * col = m_startColumn;
}

void AST::setEndPosition( int line, int col )
{
   m_endLine = line;
   m_endColumn = col;
}

void AST::getEndPosition( int* line, int* col ) const
{
    if( line )
        *line = m_endLine;
	
    if( col )
        * col = m_endColumn;
}


// ------------------------------------------------------------------------
NameAST::NameAST()
    : m_global( false )
{
}

NameAST::~NameAST()
{
}

bool NameAST::isGlobal() const
{
    return m_global;
}

void NameAST::setGlobal( bool b )
{
    m_global = b;
}

void NameAST::setNestedName( NestedNameSpecifierAST::Node& nestedName )
{
    m_nestedName = nestedName;
}

NestedNameSpecifierAST* NameAST::nestedName()
{
    return m_nestedName.get();
}
    
void NameAST::setUnqualifedName( AST::Node& unqualifiedName )
{
    m_unqualifiedName = unqualifiedName;
}

AST* NameAST::unqualifiedName()
{
    return m_unqualifiedName.get();
}

// ------------------------------------------------------------------------
DeclarationAST::DeclarationAST()
{
}

DeclarationAST::~DeclarationAST()
{
}

// ------------------------------------------------------------------------
LinkageBodyAST::LinkageBodyAST()
{
    m_declarations.setAutoDelete( true );
}

LinkageBodyAST::~LinkageBodyAST()
{
}

void LinkageBodyAST::addDeclaration( DeclarationAST::Node& ast )
{
    if( !ast.get() )
        return;
	
    m_declarations.append( ast.release() );
}

// ------------------------------------------------------------------------
LinkageSpecificationAST::LinkageSpecificationAST()
{
}

LinkageSpecificationAST::~LinkageSpecificationAST()
{
}

QString LinkageSpecificationAST::externType() const
{
    return m_externType;
}

void LinkageSpecificationAST::setExternType( const QString& type )
{
    m_externType = type;
}

LinkageBodyAST* LinkageSpecificationAST::linkageBody()
{
    return m_linkageBody.get();
}

void LinkageSpecificationAST::setLinkageBody( LinkageBodyAST::Node& linkageBody )
{
    m_linkageBody = linkageBody;
}

DeclarationAST* LinkageSpecificationAST::declaration()
{
    return m_declaration.get();
}

void LinkageSpecificationAST::setDeclaration( DeclarationAST::Node& decl )
{
    m_declaration = decl;
}
    
// ------------------------------------------------------------------------
TranslationUnitAST::TranslationUnitAST()
{
    m_declarations.setAutoDelete( true );
}

TranslationUnitAST::~TranslationUnitAST()
{
}

void TranslationUnitAST::addDeclaration( DeclarationAST::Node& ast )
{
    if( !ast.get() )
        return;
	
    m_declarations.append( ast.release() );
}

// ------------------------------------------------------------------------
NamespaceAST::NamespaceAST()
{
}

NamespaceAST::~NamespaceAST()
{
}

QString NamespaceAST::namespaceName() const
{
    return m_namespaceName;
}

void NamespaceAST::setNamespaceName( const QString& name )
{
    m_namespaceName = name;
}

LinkageBodyAST* NamespaceAST::linkageBody()
{
    return m_linkageBody.get();
}

void NamespaceAST::setLinkageBody( LinkageBodyAST::Node& linkageBody )
{
    m_linkageBody = linkageBody;
}


// ------------------------------------------------------------------------
NamespaceAliasAST::NamespaceAliasAST()
{
}

NamespaceAliasAST::~NamespaceAliasAST()
{
}

QString NamespaceAliasAST::namespaceName() const
{
    return m_namespaceName;
}

void NamespaceAliasAST::setNamespaceName( const QString& name )
{
    m_namespaceName = name;
}

NameAST* NamespaceAliasAST::aliasName()
{
    return m_aliasName.get();
}

void NamespaceAliasAST::setAliasName( NameAST::Node& name )
{
    m_aliasName = name;
}

// ------------------------------------------------------------------------
UsingAST::UsingAST()
    : m_typename( false )
{
}

UsingAST::~UsingAST()
{
}

bool UsingAST::isTypename() const
{
    return m_typename;
}

void UsingAST::setTypename( bool b )
{
    m_typename = b;
}

NameAST* UsingAST::name()
{
    return m_name.get();
}

void UsingAST::setName( NameAST::Node& name )
{
    m_name = name;
}

// ------------------------------------------------------------------------
UsingDirectiveAST::UsingDirectiveAST()
{
}

UsingDirectiveAST::~UsingDirectiveAST()
{
}

NameAST* UsingDirectiveAST::name()
{
    return m_name.get();
}

void UsingDirectiveAST::setName( NameAST::Node& name )
{
    m_name = name;
}

TypedefAST::TypedefAST()
{
}

TypedefAST::~TypedefAST()
{
}

TypeSpecifierAST* TypedefAST::typeSpec()
{
    return m_typeSpec.get();
}

void TypedefAST::setTypeSpec( TypeSpecifierAST::Node& typeSpec )
{
    m_typeSpec = typeSpec;
}

InitDeclaratorListAST* TypedefAST::initDeclaratorList()
{
    return m_initDeclaratorList.get();
}

void TypedefAST::setInitDeclaratorList( InitDeclaratorListAST::Node& initDeclaratorList )
{
    m_initDeclaratorList = initDeclaratorList;
}

// ------------------------------------------------------------------------
TemplateArgumentListAST::TemplateArgumentListAST()
{
}

TemplateArgumentListAST::~TemplateArgumentListAST()
{
}

void TemplateArgumentListAST::addArgument( AST::Node& arg )
{
    if( !arg.get() )
        return;
	
    m_arguments.append( arg.release() );
}


// ------------------------------------------------------------------------
TemplateDeclarationAST::TemplateDeclarationAST()
    : m_export( false )
{
}

TemplateDeclarationAST::~TemplateDeclarationAST()
{
}

bool TemplateDeclarationAST::isExport() const
{
    return m_export;
}

void TemplateDeclarationAST::setExport( bool b )
{
    m_export = b;
}

AST* TemplateDeclarationAST::templateParameterList()
{
    return m_templateParameterList.get();
}

void TemplateDeclarationAST::setTemplateParameterList( AST::Node& templateParameterList )
{
    m_templateParameterList = templateParameterList;
}

DeclarationAST* TemplateDeclarationAST::declaration()
{
    return m_declaration.get();
}

void TemplateDeclarationAST::setDeclaration( DeclarationAST::Node& declaration )
{
    m_declaration = declaration;
}

// ------------------------------------------------------------------------
ClassOrNamespaceNameAST::ClassOrNamespaceNameAST()
{
}

ClassOrNamespaceNameAST::~ClassOrNamespaceNameAST()
{
}

AST* ClassOrNamespaceNameAST::name()
{
    return m_name.get();
}

void ClassOrNamespaceNameAST::setName( AST::Node& name )
{
    m_name = name;
}

TemplateArgumentListAST* ClassOrNamespaceNameAST::templateArgumentList()
{
    return m_templateArgumentList.get();
}

void ClassOrNamespaceNameAST::setTemplateArgumentList( TemplateArgumentListAST::Node& templateArgumentList )
{
    m_templateArgumentList = templateArgumentList;
}

// ------------------------------------------------------------------------
NestedNameSpecifierAST::NestedNameSpecifierAST()
{
    m_classOrNamespaceNameList.setAutoDelete( true );
}

NestedNameSpecifierAST::~NestedNameSpecifierAST()
{
}

void NestedNameSpecifierAST::addClassOrNamespaceName( ClassOrNamespaceNameAST::Node& classOrNamespaceName )
{
    if( !classOrNamespaceName.get() )
        return;
	
    m_classOrNamespaceNameList.append( classOrNamespaceName.release() );
}


// ------------------------------------------------------------------------
TypeSpecifierAST::TypeSpecifierAST()
{
}

TypeSpecifierAST::~TypeSpecifierAST()
{
}


// ------------------------------------------------------------------------
ClassSpecifierAST::ClassSpecifierAST()
{
    m_declarations.setAutoDelete( true );
}

ClassSpecifierAST::~ClassSpecifierAST()
{
}

AST* ClassSpecifierAST::classKey()
{
    return m_classKey.get();
}

void ClassSpecifierAST::setClassKey( AST::Node& classKey )
{
    m_classKey = classKey;
}

NameAST* ClassSpecifierAST::name()
{
    return m_name.get();
}

void ClassSpecifierAST::setName( NameAST::Node& name )
{
    m_name = name;
}

void ClassSpecifierAST::addDeclaration( DeclarationAST::Node& declaration )
{
    if( !declaration.get() )
        return;
	
    m_declarations.append( declaration.release() );
}

// ------------------------------------------------------------------------
EnumSpecifierAST::EnumSpecifierAST()
{
    m_enumerators.setAutoDelete( true );
}

EnumSpecifierAST::~EnumSpecifierAST()
{
}

AST* EnumSpecifierAST::name()
{
    return m_name.get();
}

void EnumSpecifierAST::setName( AST::Node& name )
{
    m_name = name;
}

void EnumSpecifierAST::addEnumerator( EnumeratorAST::Node& enumerator )
{
    if( !enumerator.get() )
        return;
	
    m_enumerators.append( enumerator.release() );
}


// ------------------------------------------------------------------------
ElaboratedTypeSpecifierAST::ElaboratedTypeSpecifierAST()
{
}

ElaboratedTypeSpecifierAST::~ElaboratedTypeSpecifierAST()
{
}

AST* ElaboratedTypeSpecifierAST::kind()
{
    return m_kind.get();
}

void ElaboratedTypeSpecifierAST::setKind( AST::Node& kind )
{
    m_kind = kind;
}

NameAST* ElaboratedTypeSpecifierAST::name()
{
    return m_name.get();
}

void ElaboratedTypeSpecifierAST::setName( NameAST::Node& name )
{
    m_name = name;
}
        
// ------------------------------------------------------------------------
StatementAST::StatementAST()
{
}

StatementAST::~StatementAST()
{
}

// ------------------------------------------------------------------------
EnumeratorAST::EnumeratorAST()
{
}

EnumeratorAST::~EnumeratorAST()
{
}

AST* EnumeratorAST::id()
{
    return m_id.get();
}

void EnumeratorAST::setId( AST::Node& id )
{
    m_id = id;
}

AST* EnumeratorAST::expr()
{
    return m_expr.get();
}

void EnumeratorAST::setExpr( AST::Node& expr )
{
    m_expr = expr;
}

// ------------------------------------------------------------------------
BaseClauseAST::BaseClauseAST()
{
    m_baseSpecifiers.setAutoDelete( true );
}

BaseClauseAST::~BaseClauseAST()
{
}

void BaseClauseAST::addBaseSpecifier( BaseSpecifierAST::Node& baseSpecifier )
{
    if( !baseSpecifier.get() )
        return;
	
    m_baseSpecifiers.append( baseSpecifier.release() );
}

// ------------------------------------------------------------------------
BaseSpecifierAST::BaseSpecifierAST()
{
}

BaseSpecifierAST::~BaseSpecifierAST()
{
}

// ------------------------------------------------------------------------
SimpleDeclarationAST::SimpleDeclarationAST()
{
}

SimpleDeclarationAST::~SimpleDeclarationAST()
{
}

TypeSpecifierAST* SimpleDeclarationAST::typeSpec()
{
    return m_typeSpec.get();
}

void SimpleDeclarationAST::setTypeSpec( TypeSpecifierAST::Node& typeSpec )
{
    m_typeSpec = typeSpec;
}

InitDeclaratorListAST* SimpleDeclarationAST::initDeclaratorList()
{
    return m_initDeclaratorList.get();
}

void SimpleDeclarationAST::setInitDeclaratorList( InitDeclaratorListAST::Node& initDeclaratorList )
{
    m_initDeclaratorList = initDeclaratorList;
}

// ------------------------------------------------------------------------
InitDeclaratorListAST::InitDeclaratorListAST()
{
}

InitDeclaratorListAST::~InitDeclaratorListAST()
{
}

// ------------------------------------------------------------------------
DeclaratorAST::DeclaratorAST()
    : m_isConstMethod( false )
{
    m_ptrOpList.setAutoDelete( true );
    m_arrayDimensionList.setAutoDelete( true );
}

DeclaratorAST::~DeclaratorAST()    
{
}

DeclaratorAST* DeclaratorAST::subDeclarator()
{
    return m_subDeclarator.get();
}

void DeclaratorAST::setSubDeclarator( DeclaratorAST::Node& subDeclarator )
{
    m_subDeclarator = subDeclarator;
}

NameAST* DeclaratorAST::declaratorId()
{
    return m_declaratorId.get();
}

void DeclaratorAST::setDeclaratorId( NameAST::Node& declaratorId )
{
    m_declaratorId = declaratorId;
}

AST* DeclaratorAST::bitfieldInitialization()
{
    return m_bitfieldInitialization.get();
}

void DeclaratorAST::setBitfieldInitialization( AST::Node& bitfieldInitialization )
{
    m_bitfieldInitialization = bitfieldInitialization;
}

QPtrList<AST> DeclaratorAST::arrayDimensionList()
{
    return m_arrayDimensionList;
}

void DeclaratorAST::addArrayDimension( AST::Node& arrayDimension )
{
    if( !arrayDimension.get() )
        return;
	
    m_arrayDimensionList.append( arrayDimension.release() );
}

AST* DeclaratorAST::parameterDeclarationClause()
{
    return m_parameterDeclarationClause.get();
}

void DeclaratorAST::setParameterDeclarationClause( AST::Node& parameterDeclarationClause )
{
    m_parameterDeclarationClause = parameterDeclarationClause;
}

bool DeclaratorAST::isConstMethod()
{
    return m_isConstMethod;
}

void DeclaratorAST::setIsConstMethod( bool isConstMethod )
{
    m_isConstMethod = isConstMethod;
}

AST* DeclaratorAST::exceptionSpecification()
{
    return m_exceptionSpecification.get();
}

void DeclaratorAST::setExceptionSpecification( AST::Node& exceptionSpecification )
{
    m_exceptionSpecification = exceptionSpecification;
}

QPtrList<AST> DeclaratorAST::ptrOpList()
{
    return m_ptrOpList;
}

void DeclaratorAST::addPtrOp( AST::Node& ptrOp )
{
    if( !ptrOp.get() )
        return;
	
    m_ptrOpList.append( ptrOp.release() );
}


// --------------------------------------------------------------------------
InitDeclaratorAST::InitDeclaratorAST()
{
}

InitDeclaratorAST::~InitDeclaratorAST()
{
}

DeclaratorAST* InitDeclaratorAST::declarator()
{
    return m_declarator.get();
}

void InitDeclaratorAST::setDeclarator( DeclaratorAST::Node& declarator )
{
    m_declarator = declarator;
}

AST* InitDeclaratorAST::initializer()
{
    return m_initializer.get();
}

void InitDeclaratorAST::setInitializer( AST::Node& initializer )
{
    m_initializer = initializer;
}
