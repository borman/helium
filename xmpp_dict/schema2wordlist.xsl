<?xml version="1.0"?>
<!DOCTYPE aaa [
<!ENTITY br "
">
]>
<xsl:stylesheet 
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" 
  xmlns:xs='http://www.w3.org/2001/XMLSchema'>
  <xsl:output method="text" indent="no" />

  <xsl:strip-space elements="*"/>

  <xsl:template match="xs:schema">
    <xsl:text>#Namespace </xsl:text>
    <xsl:value-of select="@targetNamespace" />
    <xsl:text>&br;</xsl:text>

    <xsl:value-of select="@targetNamespace" />
    <xsl:text>&br;</xsl:text>

    <xsl:apply-templates />
  </xsl:template>

  <xsl:template match="xs:element">
    <xsl:if test="@name">
      <xsl:text>  </xsl:text>
      <xsl:value-of select="@name" />
      <xsl:text>&br;</xsl:text>

      <xsl:apply-templates />
    </xsl:if>
  </xsl:template>

  <xsl:template match="xs:attribute">
    <xsl:text>    </xsl:text>
    <xsl:value-of select="@name" />
    <xsl:text>&br;</xsl:text>

    <xsl:apply-templates />
  </xsl:template>

  <xsl:template match="xs:enumeration">
    <xsl:text>      </xsl:text>
    <xsl:value-of select="@value" />
    <xsl:text>&br;</xsl:text>
  </xsl:template>

</xsl:stylesheet>

