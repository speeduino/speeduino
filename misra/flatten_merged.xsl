<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output indent="yes" method="xml"/>
  
  <xsl:key name="kError" match="error" use="concat(./location/@file, '|', ./location/@line, '|', ./location/@column, '|', @id)"/>
  
  <xsl:template match="/">
	<xsl:copy>
		<xsl:apply-templates select="errors"/>
	</xsl:copy>
  </xsl:template>  

  <xsl:template match="errors">
	<xsl:copy>
		<xsl:apply-templates select="error"/>
	</xsl:copy>
  </xsl:template>    
  
  <xsl:template match="error[generate-id() = generate-id(key('kError', concat(./location/@file, '|', ./location/@line, '|', ./location/@column, '|', @id))[1])]">  
	<xsl:copy>
		<xsl:copy-of select="./location/@*"/>
		<xsl:copy-of select="@*[name()!='file0']"/>
	</xsl:copy>
  </xsl:template>
  
  <!-- Required to precisely control whitespace -->
  <xsl:template match="text()"/>
  
 </xsl:stylesheet>