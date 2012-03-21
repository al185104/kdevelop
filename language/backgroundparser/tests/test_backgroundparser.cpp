/*
 * This file is part of KDevelop
 *
 * Copyright 2012 by Sven Brauch <svenbrauch@googlemail.com>
 * Copyright 2012 by Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "test_backgroundparser.h"

#include <QTest>
#include <QElapsedTimer>
#include <QApplication>

#include <qtest_kde.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testlanguagecontroller.h>

#include <language/duchain/duchain.h>
#include <language/backgroundparser/backgroundparser.h>

#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iplugincontroller.h>

#include "testlanguagesupport.h"
#include "testparsejob.h"

QTEST_KDEMAIN(TestBackgroundparser, NoGUI)

#define QVERIFY_RETURN(statement, retval) \
do { if (!QTest::qVerify((statement), #statement, "", __FILE__, __LINE__)) return retval; } while (0)

using namespace KDevelop;

JobPlan::JobPlan()
{

}

void JobPlan::addJob(const JobPrototype& job)
{
    m_jobs << job;
}

void JobPlan::clear()
{
    m_jobs.clear();
    m_finishedJobs.clear();
    m_startedJobs.clear();
}

void JobPlan::parseJobCreated(ParseJob* job)
{
    TestParseJob* testJob = dynamic_cast<TestParseJob*>(job);
    Q_ASSERT(testJob);

    const KUrl url = testJob->document().toUrl();
    kDebug() << "assigning propierties for created job" << url;
    testJob->duration_ms = jobForUrl(url).m_duration;

    connect(testJob, SIGNAL(started(TestParseJob*)), SLOT(jobStarted(TestParseJob*)));
}

void JobPlan::jobStarted(TestParseJob* job)
{
    const KUrl url = job->document().toUrl();
    m_startedJobs.append(url);
    kDebug() << "job was started: " << url;
}

bool JobPlan::runJobs(int timeoutMS)
{
    // add parse jobs
    foreach(const JobPrototype& job, m_jobs) {
        ICore::self()->languageController()->backgroundParser()->addDocument(
            job.m_url, TopDUContext::Empty, job.m_priority, this, job.m_flags
        );
    }

    ICore::self()->languageController()->backgroundParser()->parseDocuments();

    QElapsedTimer t;
    t.start();

    while ( !t.hasExpired(timeoutMS) && m_jobs.size() != m_finishedJobs.size() ) {
        QTest::qWait(50);
    }

    QVERIFY_RETURN(m_jobs.size() == m_startedJobs.size(), false);

    QVERIFY_RETURN(m_finishedJobs.size() == m_jobs.size(), false);

    // verify they're started in the right order
    int currentBestPriority = BackgroundParser::BestPriority;
    foreach ( const KUrl& url, m_startedJobs ) {
        const JobPrototype p = jobForUrl(url);
        QVERIFY_RETURN(p.m_priority >= currentBestPriority, false);
        currentBestPriority = p.m_priority;
    }

    return true;
}

JobPrototype JobPlan::jobForUrl(const KUrl& url)
{
    foreach(const JobPrototype& job, m_jobs) {
        if (job.m_url == url) {
            return job;
        }
    }
    return JobPrototype();
}

const QVector< KUrl >& JobPlan::startedJobs()
{
    return m_startedJobs;
}

void JobPlan::updateReady(const IndexedString& url, const ReferencedTopDUContext& context)
{
    kDebug() << "update ready on " << url.toUrl();

    const JobPrototype job = jobForUrl(url.toUrl());
    QVERIFY(job.m_url.isValid());

    if (job.m_flags & ParseJob::RequiresSequentialProcessing) {
        // ensure that all jobs that respect sequential processing
        // with lower priority have been run
        foreach(const JobPrototype& otherJob, m_jobs) {
            if (otherJob.m_url == job.m_url) {
                continue;
            }
            if (otherJob.m_flags & ParseJob::RespectsSequentialProcessing &&
                otherJob.m_priority < job.m_priority)
            {
                QVERIFY(m_finishedJobs.contains(otherJob.m_url));
            }
        }
    }

    QVERIFY(!m_finishedJobs.contains(job.m_url));
    m_finishedJobs << job.m_url;
}

void TestBackgroundparser::initTestCase()
{
  AutoTestShell::init();
  TestCore::initialize(Core::NoUi);
  TestCore* core = dynamic_cast<TestCore*>(TestCore::self());

  DUChain::self()->disablePersistentStorage();

  TestLanguageController* langController = new TestLanguageController(core);
  core->setLanguageController(langController);
  langController->backgroundParser()->setThreadCount(4);

  TestLanguageSupport* testLang = new TestLanguageSupport();
  connect(testLang, SIGNAL(parseJobCreated(KDevelop::ParseJob*)),
          &m_jobPlan, SLOT(parseJobCreated(KDevelop::ParseJob*)));
  langController->addTestLanguage(testLang, QStringList() << "text/plain");
}

void TestBackgroundparser::cleanupTestCase()
{
  TestCore::shutdown();
}

void TestBackgroundparser::init()
{
    m_jobPlan.clear();
}

void TestBackgroundparser::testParseOrdering_foregroundThread()
{
    m_jobPlan.clear();
    // prove that background parsing happens with sequential flags although there is a high-priority
    // foreground thread (active document being edited, ...) running all the time.

    // the long-running high-prio job
    m_jobPlan.addJob(JobPrototype(KUrl("test_fgt_hp.txt"), -500, ParseJob::IgnoresSequentialProcessing, 630));

    // several small background jobs
    for ( int i = 0; i < 10; i++ ) {
        m_jobPlan.addJob(JobPrototype(KUrl("test_fgt_lp__" + QString::number(i) + ".txt"), i, ParseJob::FullSequentialProcessing, 40));
    }

    // not enough time if the small jobs run after the large one
    QVERIFY(m_jobPlan.runJobs(700));
}

void TestBackgroundparser::testParseOrdering_noSequentialProcessing()
{
    m_jobPlan.clear();
    for ( int i = 0; i < 20; i++ ) {
        // create jobs with no sequential processing, and different priorities
        m_jobPlan.addJob(JobPrototype(KUrl("test_nsp1__" + QString::number(i) + ".txt"), i, ParseJob::IgnoresSequentialProcessing, i));
    }
    for ( int i = 0; i < 8; i++ ) {
        // create a few more jobs with the same priority
        m_jobPlan.addJob(JobPrototype(KUrl("test_nsp2__" + QString::number(i) + ".txt"), 10, ParseJob::IgnoresSequentialProcessing, i));
    }
    QVERIFY(m_jobPlan.runJobs(1000));
}

void TestBackgroundparser::testParseOrdering_lockup()
{
    m_jobPlan.clear();
    for ( int i = 3; i > 0; i-- ) {
        // add 3 jobs which do not care about sequential processing, at 4 threads it should take no more than 1s to process them
        m_jobPlan.addJob(JobPrototype(KUrl("test" + QString::number(i) + ".txt"), i, ParseJob::IgnoresSequentialProcessing, 200));
    }
    // add one job which requires sequential processing with high priority
    m_jobPlan.addJob(JobPrototype(KUrl("test_hp.txt"), -200, ParseJob::FullSequentialProcessing, 200));
    // verify that the low-priority nonsequential jobs are run simultaneously with the other one.
    QVERIFY(m_jobPlan.runJobs(700));
}

void TestBackgroundparser::testParseOrdering_simple()
{
    m_jobPlan.clear();
    for ( int i = 20; i > 0; i-- ) {
        // the job with priority i should be at place i in the finished list
        // (lower priority value -> should be parsed first)
        ParseJob::SequentialProcessingFlags flags = ParseJob::FullSequentialProcessing;
        m_jobPlan.addJob(JobPrototype(KUrl("test" + QString::number(i) + ".txt"),
                                      i, flags));
    }
    // also add a few jobs which ignore the processing
    for ( int i = 0; i < 5; ++i ) {
        m_jobPlan.addJob(JobPrototype(KUrl("test2-" + QString::number(i) + ".txt"),
                                       BackgroundParser::NormalPriority,
                                       ParseJob::IgnoresSequentialProcessing));
    }

    QVERIFY(m_jobPlan.runJobs(1000));
}

void TestBackgroundparser::benchmark()
{
    const int jobs = 1000;

    QVector<KUrl> jobUrls;
    jobUrls.reserve(jobs);
    for ( int i = 0; i < jobs; ++i ) {
        jobUrls << KUrl("test" + QString::number(i) + ".txt");
    }

    QBENCHMARK {
        foreach ( const KUrl& url, jobUrls ) {
            ICore::self()->languageController()->backgroundParser()->addDocument(url);
        }

        ICore::self()->languageController()->backgroundParser()->parseDocuments();

        while ( ICore::self()->languageController()->backgroundParser()->queuedCount() ) {
            QTest::qWait(50);
        }
    }
}

#include "test_backgroundparser.moc"
