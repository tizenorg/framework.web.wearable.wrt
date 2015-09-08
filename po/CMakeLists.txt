# Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

# 78 language(*.po)
SET(POFILES
ar.po  as.po  az.po  bg.po  bn.po  ca.po  cs.po  da.po  de.po  el_GR.po  en.po  en_PH.po  en_US.po  es_ES.po  es_US.po  et.po  eu.po  fa.po  fi.po  fr.po  fr_CA.po
ga.po  gl.po  gu.po  he.po  hi.po  hr.po  hu.po  hy.po  id.po  is.po  it_IT.po  ja_JP.po  ka.po  kk.po  km.po  kn.po  ko_KR.po  ky_KG.po  lo.po  lt.po  lv.po  mk.po
ml.po  mn_MN.po  mr.po  ms.po  my.po  nb.po  ne.po  nl.po  or.po  pa.po  pl.po  pt_BR.po  pt_PT.po  ro.po  ru_RU.po  si.po  sk.po  sl.po  sq.po  sr.po  sv.po  ta.po
te.po  tg_TJ.po  th.po  tk_TM.po  tl.po  tr_TR.po  uk.po  ur.po  uz.po  vi.po  zh_CN.po  zh_HK.po  zh_TW.po
)

SET(MSGFMT "/usr/bin/msgfmt")
SET(LOCALE_DIR "/usr/share/locale/")

FOREACH(pofile ${POFILES})
	SET(pofile ${CMAKE_CURRENT_SOURCE_DIR}/${pofile})
	MESSAGE("PO: ${pofile}")
	GET_FILENAME_COMPONENT(absPofile ${pofile} ABSOLUTE)
	GET_FILENAME_COMPONENT(lang ${absPofile} NAME_WE)
	SET(moFile ${CMAKE_CURRENT_BINARY_DIR}/${lang}.mo)
        ADD_CUSTOM_COMMAND( OUTPUT ${moFile}
            COMMAND ${MSGFMT} -o ${moFile} ${absPofile}
            DEPENDS ${absPofile} )
        INSTALL(FILES ${moFile}
            DESTINATION ${LOCALE_DIR}/${lang}/LC_MESSAGES
            RENAME  ${PROJECT_NAME}.mo)
        SET(moFiles ${moFiles} ${moFile})
ENDFOREACH(pofile)

MESSAGE(".mo files: ${moFiles}")
ADD_CUSTOM_TARGET(po ALL DEPENDS ${moFiles})
