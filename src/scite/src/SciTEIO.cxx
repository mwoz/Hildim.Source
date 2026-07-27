bool SciTEBase::strstrRegExp(char *text, const char *sub, void *pRegExp, GrepFlags gf, const char* charsAccented)
{//��� ������������������ luaState ���������� ����� regex_search ��� ����������� ������� ������� ��������� ��������� � �����
	if (!pRegExp) {