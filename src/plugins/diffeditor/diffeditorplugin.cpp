#include <QFutureWatcher>
#include <coreplugin/progressmanager/progressmanager.h>
#include <utils/mapreduce.h>
class ReloadInput {
public:
    QString leftText;
    QString rightText;
    DiffFileInfo leftFileInfo;
    DiffFileInfo rightFileInfo;
    FileData::FileOperation fileOperation = FileData::ChangeFile;
};

class DiffFile
{
public:
    DiffFile(bool ignoreWhitespace, int contextLineCount)
        : m_contextLineCount(contextLineCount),
          m_ignoreWhitespace(ignoreWhitespace)
    {}

    void operator()(QFutureInterface<FileData> &futureInterface,
                    const ReloadInput &reloadInfo) const
    {
        Differ differ(&futureInterface);
        const QList<Diff> diffList = differ.cleanupSemantics(
                    differ.diff(reloadInfo.leftText, reloadInfo.rightText));

        QList<Diff> leftDiffList;
        QList<Diff> rightDiffList;
        Differ::splitDiffList(diffList, &leftDiffList, &rightDiffList);
        QList<Diff> outputLeftDiffList;
        QList<Diff> outputRightDiffList;

        if (m_ignoreWhitespace) {
            const QList<Diff> leftIntermediate
                    = Differ::moveWhitespaceIntoEqualities(leftDiffList);
            const QList<Diff> rightIntermediate
                    = Differ::moveWhitespaceIntoEqualities(rightDiffList);
            Differ::ignoreWhitespaceBetweenEqualities(leftIntermediate, rightIntermediate,
                                                      &outputLeftDiffList, &outputRightDiffList);
        } else {
            outputLeftDiffList = leftDiffList;
            outputRightDiffList = rightDiffList;
        }

        const ChunkData chunkData = DiffUtils::calculateOriginalData(
                    outputLeftDiffList, outputRightDiffList);
        FileData fileData = DiffUtils::calculateContextData(chunkData, m_contextLineCount, 0);
        fileData.leftFileInfo = reloadInfo.leftFileInfo;
        fileData.rightFileInfo = reloadInfo.rightFileInfo;
        fileData.fileOperation = reloadInfo.fileOperation;
        futureInterface.reportResult(fileData);
    }

private:
    const int m_contextLineCount;
    const bool m_ignoreWhitespace;
};

    ~DiffFilesController();
    void reload() final;
    virtual QList<ReloadInput> reloadInputList() const = 0;
private:
    void reloaded();
    void cancelReload();

    QFutureWatcher<FileData> m_futureWatcher;
{
    connect(&m_futureWatcher, &QFutureWatcher<FileData>::finished,
            this, &DiffFilesController::reloaded);
}
DiffFilesController::~DiffFilesController()
    cancelReload();
}
void DiffFilesController::reload()
{
    cancelReload();
    m_futureWatcher.setFuture(Utils::map(reloadInputList(),
                                         DiffFile(ignoreWhitespace(),
                                                  contextLineCount())));

    Core::ProgressManager::addTask(m_futureWatcher.future(),
                                   tr("Calculating diff"), "DiffEditor");
}

void DiffFilesController::reloaded()
{
    const bool success = !m_futureWatcher.future().isCanceled();
    const QList<FileData> fileDataList = success
            ? m_futureWatcher.future().results() : QList<FileData>();

    setDiffFiles(fileDataList);
    reloadFinished(success);
}
void DiffFilesController::cancelReload()
{
    if (m_futureWatcher.future().isRunning()) {
        m_futureWatcher.future().cancel();
        m_futureWatcher.setFuture(QFuture<FileData>());
    }
    QList<ReloadInput> reloadInputList() const final;
QList<ReloadInput> DiffCurrentFileController::reloadInputList() const
    QList<ReloadInput> result;
        ReloadInput reloadInput;
        reloadInput.leftText = leftText;
        reloadInput.rightText = rightText;
        reloadInput.leftFileInfo.fileName = m_fileName;
        reloadInput.rightFileInfo.fileName = m_fileName;
        reloadInput.leftFileInfo.typeInfo = tr("Saved");
        reloadInput.rightFileInfo.typeInfo = tr("Modified");
        reloadInput.rightFileInfo.patchBehaviour = DiffFileInfo::PatchEditor;
            reloadInput.fileOperation = FileData::NewFile;
        result << reloadInput;
    return result;
    QList<ReloadInput> reloadInputList() const final;
QList<ReloadInput> DiffOpenFilesController::reloadInputList() const
    QList<ReloadInput> result;
    const QList<IDocument *> openedDocuments = DocumentModel::openedDocuments();
    for (IDocument *doc : openedDocuments) {
            ReloadInput reloadInput;
            reloadInput.leftText = leftText;
            reloadInput.rightText = rightText;
            reloadInput.leftFileInfo.fileName = fileName;
            reloadInput.rightFileInfo.fileName = fileName;
            reloadInput.leftFileInfo.typeInfo = tr("Saved");
            reloadInput.rightFileInfo.typeInfo = tr("Modified");
            reloadInput.rightFileInfo.patchBehaviour = DiffFileInfo::PatchEditor;
                reloadInput.fileOperation = FileData::NewFile;
            result << reloadInput;
    return result;
    QList<ReloadInput> reloadInputList() const final;
QList<ReloadInput> DiffModifiedFilesController::reloadInputList() const
    QList<ReloadInput> result;
    for (const QString &fileName : m_fileNames) {
            ReloadInput reloadInput;
            reloadInput.leftText = leftText;
            reloadInput.rightText = rightText;
            reloadInput.leftFileInfo.fileName = fileName;
            reloadInput.rightFileInfo.fileName = fileName;
            reloadInput.leftFileInfo.typeInfo = tr("Saved");
            reloadInput.rightFileInfo.typeInfo = tr("Modified");
            reloadInput.rightFileInfo.patchBehaviour = DiffFileInfo::PatchEditor;
                reloadInput.fileOperation = FileData::NewFile;
            result << reloadInput;
    return result;
    QList<ReloadInput> reloadInputList() const final;
QList<ReloadInput> DiffExternalFilesController::reloadInputList() const
    ReloadInput reloadInput;
    reloadInput.leftText = leftText;
    reloadInput.rightText = rightText;
    reloadInput.leftFileInfo.fileName = m_leftFileName;
    reloadInput.rightFileInfo.fileName = m_rightFileName;
        reloadInput.fileOperation = FileData::NewFile;
        reloadInput.fileOperation = FileData::DeleteFile;
    QList<ReloadInput> result;
        result << reloadInput;
    return result;
void DiffEditorServiceImpl::diffFiles(const QString &leftFileName, const QString &rightFileName)
{
    const QString documentId = Constants::DIFF_EDITOR_PLUGIN
            + QLatin1String(".DiffFiles.") + leftFileName + QLatin1Char('.') + rightFileName;
    const QString title = tr("Diff Files");
    auto const document = qobject_cast<DiffEditorDocument *>(
                DiffEditorController::findOrCreateDocument(documentId, title));
    if (!document)
        return;

    if (!DiffEditorController::controller(document))
        new DiffExternalFilesController(document, leftFileName, rightFileName);
    EditorManager::activateEditorForDocument(document);
    document->reload();
}

    const QString documentId = Constants::DIFF_EDITOR_PLUGIN
            + QLatin1String(".DiffModifiedFiles");
    diffCurrentFileCommand->setDefaultKeySequence(QKeySequence(useMacShortcuts ? tr("Meta+H") : tr("Ctrl+H")));
    diffOpenFilesCommand->setDefaultKeySequence(QKeySequence(useMacShortcuts ? tr("Meta+Shift+H") : tr("Ctrl+Shift+H")));
    new DiffEditorFactory(this);
    new DiffEditorServiceImpl(this);
    const QString documentId = Constants::DIFF_EDITOR_PLUGIN
            + QLatin1String(".Diff.") + fileName;
    const QString documentId = Constants::DIFF_EDITOR_PLUGIN
            + QLatin1String(".DiffOpenFiles");
    const QString documentId = Constants::DIFF_EDITOR_PLUGIN
            + QLatin1String(".DiffExternalFiles.") + fileName1 + QLatin1Char('.') + fileName2;
    const QString fileName = "a.txt";
    const QString header = "--- " + fileName + "\n+++ " + fileName + '\n';
    QString patchText = header + "@@ -1,2 +1,1 @@\n"
                                 "-ABCD\n"
                                 " EFGH\n";
    patchText = header + "@@ -1,2 +1,1 @@\n"
                         "-ABCD\n"
                         " EFGH\n"
                         "\\ No newline at end of file\n";
    patchText = header + "@@ -1,1 +1,1 @@\n"
                         "-ABCD\n"
                         "+ABCD\n"
                         "\\ No newline at end of file\n";
    patchText = header + "@@ -1,2 +1,1 @@\n"
                         " ABCD\n"
                         "-\n";
    patchText = header + "@@ -1,2 +1,1 @@\n"
                         "-ABCD\n"
                         "-\n"
                         "+ABCD\n"
                         "\\ No newline at end of file\n";
    patchText = header + "@@ -1,1 +1,1 @@\n"
                         "-ABCD\n"
                         "\\ No newline at end of file\n"
                         "+ABCD\n";
    patchText = header + "@@ -1,1 +1,2 @@\n"
                         " ABCD\n"
                         "+\n";
    patchText = header + "@@ -1,1 +1,1 @@\n"
                         "-ABCD\n"
                         "+EFGH\n";
    patchText = header + "@@ -1,2 +1,2 @@\n"
                         "-ABCD\n"
                         "+EFGH\n"
                         " \n";
    patchText = header + "@@ -1,1 +1,1 @@\n"
                         "-ABCD\n"
                         "\\ No newline at end of file\n"
                         "+EFGH\n"
                         "\\ No newline at end of file\n";
    patchText = header + "@@ -1,1 +1,1 @@\n"
                         "-ABCD\n"
                         "+EFGH\n";
    patchText = header + "@@ -1,2 +1,2 @@\n"
                         "-ABCD\n"
                         "+EFGH\n"
                         " IJKL\n"
                         "\\ No newline at end of file\n";
    patchText = header + "@@ -1,2 +1,2 @@\n"
                         "-ABCD\n"
                         "+EFGH\n"
                         " IJKL\n";
    patchText = header + "@@ -1,1 +1,3 @@\n"
                         " ABCD\n"
                         "+\n"
                         "+EFGH\n"
                         "\\ No newline at end of file\n";
    const QString fileName = "a.txt";
    const QString result = DiffUtils::makePatch(sourceChunk, fileName, fileName, lastChunk);
        QCOMPARE(resultFileData.leftFileInfo.fileName, fileName);
        QCOMPARE(resultFileData.rightFileInfo.fileName, fileName);
    QString patch = "diff --git a/src/plugins/diffeditor/diffeditor.cpp b/src/plugins/diffeditor/diffeditor.cpp\n"
                    "index eab9e9b..082c135 100644\n"
                    "--- a/src/plugins/diffeditor/diffeditor.cpp\n"
                    "+++ b/src/plugins/diffeditor/diffeditor.cpp\n"
                    "@@ -187,9 +187,6 @@ void DiffEditor::ctor()\n"
                    "     m_controller = m_document->controller();\n"
                    "     m_guiController = new DiffEditorGuiController(m_controller, this);\n"
                    " \n"
                    "-//    m_sideBySideEditor->setDiffEditorGuiController(m_guiController);\n"
                    "-//    m_unifiedEditor->setDiffEditorGuiController(m_guiController);\n"
                    "-\n"
                    "     connect(m_controller, SIGNAL(cleared(QString)),\n"
                    "             this, SLOT(slotCleared(QString)));\n"
                    "     connect(m_controller, SIGNAL(diffContentsChanged(QList<DiffEditorController::DiffFilesContents>,QString)),\n"
                    "diff --git a/src/plugins/diffeditor/diffutils.cpp b/src/plugins/diffeditor/diffutils.cpp\n"
                    "index 2f641c9..f8ff795 100644\n"
                    "--- a/src/plugins/diffeditor/diffutils.cpp\n"
                    "+++ b/src/plugins/diffeditor/diffutils.cpp\n"
                    "@@ -464,5 +464,12 @@ QString DiffUtils::makePatch(const ChunkData &chunkData,\n"
                    "     return diffText;\n"
                    " }\n"
                    " \n"
                    "+FileData DiffUtils::makeFileData(const QString &patch)\n"
                    "+{\n"
                    "+    FileData fileData;\n"
                    "+\n"
                    "+    return fileData;\n"
                    "+}\n"
                    "+\n"
                    " } // namespace Internal\n"
                    " } // namespace DiffEditor\n"
                    "diff --git a/new b/new\n"
                    "new file mode 100644\n"
                    "index 0000000..257cc56\n"
                    "--- /dev/null\n"
                    "+++ b/new\n"
                    "@@ -0,0 +1 @@\n"
                    "+foo\n"
                    "diff --git a/deleted b/deleted\n"
                    "deleted file mode 100644\n"
                    "index 257cc56..0000000\n"
                    "--- a/deleted\n"
                    "+++ /dev/null\n"
                    "@@ -1 +0,0 @@\n"
                    "-foo\n"
                    "diff --git a/empty b/empty\n"
                    "new file mode 100644\n"
                    "index 0000000..e69de29\n"
                    "diff --git a/empty b/empty\n"
                    "deleted file mode 100644\n"
                    "index e69de29..0000000\n"
                    "diff --git a/file a.txt b/file b.txt\n"
                    "similarity index 99%\n"
                    "copy from file a.txt\n"
                    "copy to file b.txt\n"
                    "index 1234567..9876543\n"
                    "--- a/file a.txt\n"
                    "+++ b/file b.txt\n"
                    "@@ -20,3 +20,3 @@\n"
                    " A\n"
                    "-B\n"
                    "+C\n"
                    " D\n"
                    "diff --git a/file a.txt b/file b.txt\n"
                    "similarity index 99%\n"
                    "rename from file a.txt\n"
                    "rename to file b.txt\n"
                    "diff --git a/file.txt b/file.txt\n"
                    "old mode 100644\n"
                    "new mode 100755\n"
                    "index 1234567..9876543\n"
                    "--- a/file.txt\n"
                    "+++ b/file.txt\n"
                    "@@ -20,3 +20,3 @@\n"
                    " A\n"
                    "-B\n"
                    "+C\n"
                    " D\n"
                    ;
    fileData1.leftFileInfo = DiffFileInfo("src/plugins/diffeditor/diffeditor.cpp", "eab9e9b");
    fileData1.rightFileInfo = DiffFileInfo("src/plugins/diffeditor/diffeditor.cpp", "082c135");
    fileData3.leftFileInfo = DiffFileInfo("new", "0000000");
    fileData3.rightFileInfo = DiffFileInfo("new", "257cc56");
    fileData4.leftFileInfo = DiffFileInfo("deleted", "257cc56");
    fileData4.rightFileInfo = DiffFileInfo("deleted", "0000000");
    fileData5.leftFileInfo = DiffFileInfo("empty", "0000000");
    fileData5.rightFileInfo = DiffFileInfo("empty", "e69de29");
    fileData6.leftFileInfo = DiffFileInfo("empty", "e69de29");
    fileData6.rightFileInfo = DiffFileInfo("empty", "0000000");
    fileData7.leftFileInfo = DiffFileInfo("file a.txt", "1234567");
    fileData7.rightFileInfo = DiffFileInfo("file b.txt", "9876543");
    fileData8.leftFileInfo = DiffFileInfo("file a.txt");
    fileData8.rightFileInfo = DiffFileInfo("file b.txt");
    fileData9.leftFileInfo = DiffFileInfo("file.txt", "1234567");
    fileData9.rightFileInfo = DiffFileInfo("file.txt", "9876543");
    patch = "diff --git a/file foo.txt b/file foo.txt\n"
            "index 1234567..9876543 100644\n"
            "--- a/file foo.txt\n"
            "+++ b/file foo.txt\n"
            "@@ -50,4 +50,5 @@ void DiffEditor::ctor()\n"
            " A\n"
            " B\n"
            " C\n"
            "+\n";

    fileData1.leftFileInfo = DiffFileInfo("file foo.txt", "1234567");
    fileData1.rightFileInfo = DiffFileInfo("file foo.txt", "9876543");
    patch = "diff --git a/file foo.txt b/file foo.txt\n"
            "index 1234567..9876543 100644\n"
            "--- a/file foo.txt\n"
            "+++ b/file foo.txt\n"
            "@@ -1,1 +1,1 @@\n"
            "-ABCD\n"
            "\\ No newline at end of file\n"
            "+ABCD\n";

    fileData1.leftFileInfo = DiffFileInfo("file foo.txt", "1234567");
    fileData1.rightFileInfo = DiffFileInfo("file foo.txt", "9876543");
    patch = "diff --git a/difftest.txt b/difftest.txt\n"
            "index 1234567..9876543 100644\n"
            "--- a/difftest.txt\n"
            "+++ b/difftest.txt\n"
            "@@ -2,5 +2,5 @@ void func()\n"
            " A\n"
            " B\n"
            "-C\n"
            "+Z\n"
            " D\n"
            " \n"
            "@@ -9,2 +9,4 @@ void OtherFunc()\n"
            " \n"
            " D\n"
            "+E\n"
            "+F\n"
            ;

    fileData1.leftFileInfo = DiffFileInfo("difftest.txt", "1234567");
    fileData1.rightFileInfo = DiffFileInfo("difftest.txt", "9876543");
    patch = "diff --git a/file foo.txt b/file foo.txt\n"
            "index 1234567..9876543 100644\n"
            "--- a/file foo.txt\n"
            "+++ b/file foo.txt\n"
            "@@ -1,1 +1,3 @@ void DiffEditor::ctor()\n"
            " ABCD\n"
            "+\n"
            "+EFGH\n"
            "\\ No newline at end of file\n";

    fileData1.leftFileInfo = DiffFileInfo("file foo.txt", "1234567");
    fileData1.rightFileInfo = DiffFileInfo("file foo.txt", "9876543");
    patch = "diff --git a/src/plugins/texteditor/basetextdocument.h b/src/plugins/texteditor/textdocument.h\n"
            "similarity index 100%\n"
            "rename from src/plugins/texteditor/basetextdocument.h\n"
            "rename to src/plugins/texteditor/textdocument.h\n"
            "diff --git a/src/plugins/texteditor/basetextdocumentlayout.cpp b/src/plugins/texteditor/textdocumentlayout.cpp\n"
            "similarity index 79%\n"
            "rename from src/plugins/texteditor/basetextdocumentlayout.cpp\n"
            "rename to src/plugins/texteditor/textdocumentlayout.cpp\n"
            "index 0121933..01cc3a0 100644\n"
            "--- a/src/plugins/texteditor/basetextdocumentlayout.cpp\n"
            "+++ b/src/plugins/texteditor/textdocumentlayout.cpp\n"
            "@@ -2,5 +2,5 @@ void func()\n"
            " A\n"
            " B\n"
            "-C\n"
            "+Z\n"
            " D\n"
            " \n"
            ;
    fileData1.leftFileInfo = DiffFileInfo("src/plugins/texteditor/basetextdocument.h");
    fileData1.rightFileInfo = DiffFileInfo("src/plugins/texteditor/textdocument.h");
    fileData2.leftFileInfo = DiffFileInfo("src/plugins/texteditor/basetextdocumentlayout.cpp", "0121933");
    fileData2.rightFileInfo = DiffFileInfo("src/plugins/texteditor/textdocumentlayout.cpp", "01cc3a0");
    patch = "diff --git a/src/shared/qbs b/src/shared/qbs\n"
            "--- a/src/shared/qbs\n"
            "+++ b/src/shared/qbs\n"
            "@@ -1 +1 @@\n"
            "-Subproject commit eda76354077a427d692fee05479910de31040d3f\n"
            "+Subproject commit eda76354077a427d692fee05479910de31040d3f-dirty\n"
            ;
    fileData1.leftFileInfo = DiffFileInfo("src/shared/qbs");
    fileData1.rightFileInfo = DiffFileInfo("src/shared/qbs");
    //////////////
    patch = "diff --git a/demos/arthurplugin/arthurplugin.pro b/demos/arthurplugin/arthurplugin.pro\n"
            "new file mode 100644\n"
            "index 0000000..c5132b4\n"
            "--- /dev/null\n"
            "+++ b/demos/arthurplugin/arthurplugin.pro\n"
            "@@ -0,0 +1 @@\n"
            "+XXX\n"
            "diff --git a/demos/arthurplugin/bg1.jpg b/demos/arthurplugin/bg1.jpg\n"
            "new file mode 100644\n"
            "index 0000000..dfc7cee\n"
            "Binary files /dev/null and b/demos/arthurplugin/bg1.jpg differ\n"
            "diff --git a/demos/arthurplugin/flower.jpg b/demos/arthurplugin/flower.jpg\n"
            "new file mode 100644\n"
            "index 0000000..f8e022c\n"
            "Binary files /dev/null and b/demos/arthurplugin/flower.jpg differ\n"
            ;

    fileData1 = FileData();
    fileData1.leftFileInfo = DiffFileInfo("demos/arthurplugin/arthurplugin.pro", "0000000");
    fileData1.rightFileInfo = DiffFileInfo("demos/arthurplugin/arthurplugin.pro", "c5132b4");
    fileData1.fileOperation = FileData::NewFile;
    chunkData1 = ChunkData();
    chunkData1.leftStartingLineNumber = -1;
    chunkData1.rightStartingLineNumber = 0;
    rows1.clear();
    rows1 << RowData(TextLineData::Separator, _("XXX"));
    rows1 << RowData(TextLineData::Separator, TextLineData(TextLineData::TextLine));
    chunkData1.rows = rows1;
    fileData1.chunks << chunkData1;
    fileData2 = FileData();
    fileData2.leftFileInfo = DiffFileInfo("demos/arthurplugin/bg1.jpg", "0000000");
    fileData2.rightFileInfo = DiffFileInfo("demos/arthurplugin/bg1.jpg", "dfc7cee");
    fileData2.fileOperation = FileData::NewFile;
    fileData2.binaryFiles = true;
    fileData3 = FileData();
    fileData3.leftFileInfo = DiffFileInfo("demos/arthurplugin/flower.jpg", "0000000");
    fileData3.rightFileInfo = DiffFileInfo("demos/arthurplugin/flower.jpg", "f8e022c");
    fileData3.fileOperation = FileData::NewFile;
    fileData3.binaryFiles = true;

    QList<FileData> fileDataList8;
    fileDataList8 << fileData1 << fileData2 << fileData3;

    QTest::newRow("Binary files") << patch
                                  << fileDataList8;

    //////////////
    patch = "diff --git a/script.sh b/script.sh\n"
            "old mode 100644\n"
            "new mode 100755\n"
            ;

    fileData1 = FileData();
    fileData1.leftFileInfo = DiffFileInfo("script.sh");
    fileData1.rightFileInfo = DiffFileInfo("script.sh");
    fileData1.fileOperation = FileData::ChangeMode;

    QList<FileData> fileDataList9;
    fileDataList9 << fileData1;

    QTest::newRow("Mode change") << patch << fileDataList9;

    patch = "Index: src/plugins/subversion/subversioneditor.cpp\n"
            "===================================================================\n"
            "--- src/plugins/subversion/subversioneditor.cpp\t(revision 0)\n"
            "+++ src/plugins/subversion/subversioneditor.cpp\t(revision 0)\n"
            "@@ -0,0 +125 @@\n\n";
    fileData1.leftFileInfo = DiffFileInfo("src/plugins/subversion/subversioneditor.cpp");
    fileData1.rightFileInfo = DiffFileInfo("src/plugins/subversion/subversioneditor.cpp");
    QList<FileData> fileDataList21;
    fileDataList21 << fileData1;
                                    << fileDataList21;
    patch = "Index: src/plugins/subversion/subversioneditor.cpp\n"
            "===================================================================\n"
            "--- src/plugins/subversion/subversioneditor.cpp\t(revision 42)\n"
            "+++ src/plugins/subversion/subversioneditor.cpp\t(working copy)\n"
            "@@ -1,125 +0,0 @@\n\n";
    fileData1.leftFileInfo = DiffFileInfo("src/plugins/subversion/subversioneditor.cpp");
    fileData1.rightFileInfo = DiffFileInfo("src/plugins/subversion/subversioneditor.cpp");
    QList<FileData> fileDataList22;
    fileDataList22 << fileData1;
                                        << fileDataList22;
    patch = "Index: src/plugins/subversion/subversioneditor.cpp\n"
            "===================================================================\n"
            "--- src/plugins/subversion/subversioneditor.cpp\t(revision 42)\n"
            "+++ src/plugins/subversion/subversioneditor.cpp\t(working copy)\n"
            "@@ -120,7 +120,7 @@\n\n";
    fileData1.leftFileInfo = DiffFileInfo("src/plugins/subversion/subversioneditor.cpp");
    fileData1.rightFileInfo = DiffFileInfo("src/plugins/subversion/subversioneditor.cpp");
    QList<FileData> fileDataList23;
    fileDataList23 << fileData1;
                                       << fileDataList23;
    const QList<FileData> &result = DiffUtils::readPatch(sourcePatch, &ok);
    QCOMPARE(result.count(), fileDataList.count());