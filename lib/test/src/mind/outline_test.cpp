/*
 outline_test.cpp     MindForger test

 Copyright (C) 2016-2018 Martin Dvorak <martin.dvorak@mindforger.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../../../src/gear/file_utils.h"
#include "../../../src/model/outline.h"
#include "../../../src/model/outline_type.h"
#include "../../../src/model/note.h"
#include "../../../src/model/tag.h"
#include "../../../src/model/stencil.h"
#include "../../../src/model/resource_types.h"
#include "../../../src/mind/mind.h"
#include "../../../src/install/installer.h"

using namespace std;

void dumpOutline(m8r::Outline*& outline);

TEST(OutlineTestCase, NewAndDeleteOutline) {
    string repositoryDir{"/tmp/mf-unit-repository"};
    m8r::removeDirectoryRecursively(repositoryDir.c_str());
    m8r::Installer installer{};
    installer.createEmptyMindForgerRepository(repositoryDir);
    string oFile{repositoryDir+"/memory/outline.md"};
    string oContent{"# Test Outline\n\nOutline text.\n\n## Note 1\nNote 1 text.\n"};
    m8r::stringToFile(oFile,oContent);

    m8r::Configuration& config = m8r::Configuration::getInstance();
    config.clear();
    config.setConfigFilePath("/tmp/cfg-otc-nado.md");
    config.setActiveRepository(config.addRepository(m8r::RepositoryIndexer::getRepositoryForPath(repositoryDir)));
    m8r::Mind mind{config};
    mind.learn();
    mind.think().get();

    EXPECT_EQ(mind.remind().getOutlinesCount(), 1);
    EXPECT_EQ(mind.remind().getNotesCount(), 1);

    // just delete 1 outline w/ 1 note from memory (and check w/ Valgrind)
    mind.outlineForget(mind.remind().getOutlines()[0]->getKey());

    EXPECT_EQ(mind.remind().getOutlinesCount(), 0);
    EXPECT_EQ(mind.remind().getNotesCount(), 0);
}

TEST(OutlineTestCase, NewOutlineFromStencil) {
    // prepare M8R repository and let the mind think...
    string repositoryDir{"/tmp/mf-unit-repository-o"};
    m8r::removeDirectoryRecursively(repositoryDir.c_str());
    m8r::Installer installer{};
    installer.createEmptyMindForgerRepository(repositoryDir);
    string stencilFile{repositoryDir+"/stencils/notebooks/o-s.md"};
    string stencilContent{"# Stencil Test Outline\n\nOutline text.\n\n## Stencil Note 1\nNote 1 text.\n\n##Stencil Note 2\nNote 2 text.\n"};
    m8r::stringToFile(stencilFile,stencilContent);

    m8r::Configuration& config = m8r::Configuration::getInstance();
    config.clear();
    config.setConfigFilePath("/tmp/cfg-otc-nofs.md");
    config.setActiveRepository(config.addRepository(m8r::RepositoryIndexer::getRepositoryForPath(repositoryDir)));
    m8r::Mind mind{config};
    m8r::Memory& memory = mind.remind();
    mind.learn();
    mind.think().get();

    // create Outline using a stencil from MEMORY
    vector<const m8r::Tag*> tags{};
    tags.push_back(mind.ontology().findOrCreateTag(m8r::Tag::KeyCool()));
    vector<m8r::Stencil*>& stencils = memory.getStencils();
    m8r::Stencil* stencil = stencils.at(0);
    cout << endl << "Loaded Outline STENCILS: " << stencils.size();
    // IMPROVE constructor call is WRONG > complete parameters
    string name{"MIND's stencil Outline"};
    mind.outlineNew(
                &name,
                mind.ontology().findOrCreateOutlineType(m8r::OutlineType::KeyOutline()),
                1,
                2,
                55,
                &tags,
                nullptr,
                stencil);

    // create stencil MANUALLY (stencil file does NOT have to exist)
    unique_ptr<m8r::Stencil> outlineStencil{
        new m8r::Stencil{
            string{repositoryDir+"/stencils/outlines/grow.md"},
            string{"Manual stencil"}}};
    // IMPROVE constructor call is WRONG > complete parameters
    name.assign("MANUAL stencil Outline");
    tags.clear();
    tags.push_back(mind.ontology().findOrCreateTag(m8r::Tag::KeyImportant()));
    mind.outlineNew(
                &name,
                mind.ontology().findOrCreateOutlineType(m8r::OutlineType::KeyGrow()),
                3,
                5,
                66,
                &tags,
                nullptr,
                outlineStencil.get());

    // asserts
    EXPECT_EQ(mind.remind().getOutlinesCount(), 2);
    EXPECT_EQ(mind.remind().getNotesCount(), 3);
}

TEST(OutlineTestCase, CloneOutline) {
    // prepare M8R repository and let the mind think...
    string repositoryDir{"/tmp/mf-unit-repository-o"};
    m8r::removeDirectoryRecursively(repositoryDir.c_str());
    m8r::Installer installer{};
    installer.createEmptyMindForgerRepository(repositoryDir);
    string oFile{repositoryDir+"/memory/o.md"};
    string oContent{
        "# Note Operations Test Outline"
        "\nOutline text."
        "\n"
        "\n# 1"
        "\nT1."
        "\n"
        "\n# 2"
        "\nT2."
        "\n"
        "\n# 3"
        "\nT3."
        "\n"
        "\n## 33"
        "\nT33."
        "\n"
        "\n### 333"
        "\nT333."
        "\n"
        "\n# 4"
        "\nT4."
        "\n"
        "\n## 44"
        "\nT44."
        "\n"
        "\n# 5"
        "\nT5."
        "\n"
        "\n# 6"
        "\nT6."
        "\n"
        "\n"};
    m8r::stringToFile(oFile,oContent);

    m8r::Configuration& config = m8r::Configuration::getInstance();
    config.clear();
    config.setConfigFilePath("/tmp/cfg-otc-co.md");
    config.setActiveRepository(config.addRepository(m8r::RepositoryIndexer::getRepositoryForPath(repositoryDir)));
    m8r::Mind mind{config};
    m8r::Memory& memory = mind.remind();
    mind.learn();
    mind.think().get();


    // test
    vector<m8r::Outline*> outlines = memory.getOutlines();
    m8r::Outline* o = outlines.at(0);
    m8r::Outline* c = mind.outlineClone(o->getKey());

    // asserts
    EXPECT_TRUE(o->getFormat()==m8r::MarkdownDocument::Format::MINDFORGER);
    EXPECT_TRUE(c != nullptr);
    EXPECT_TRUE(c->getFormat()==m8r::MarkdownDocument::Format::MINDFORGER);
    EXPECT_EQ(mind.remind().getOutlinesCount(), 2);
    EXPECT_EQ(mind.remind().getNotesCount(), 18);
    cout << "O key: " << o->getKey() << endl;
    cout << "C key: " << c->getKey() << endl;
    EXPECT_NE(o->getKey(), c->getKey());
    EXPECT_EQ(o->getName(), "Note Operations Test Outline");
    EXPECT_EQ(c->getName(), "Copy of Note Operations Test Outline");
    EXPECT_EQ(o->getDescription().size(), c->getDescription().size());
    EXPECT_NE(o->getModified(), c->getModified());
}
