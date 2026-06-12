#include "Observer.h"
#include "Subject.h"
#include <algorithm>

namespace dae
{
    Observer::~Observer()
    {
        for (auto subject : m_subjects)
        {
            subject->RemoveObserverInternal(this);
        }
    }

    void Observer::AddSubjectInternal(Subject* subject)
    {
        m_subjects.push_back(subject);
    }

    void Observer::RemoveSubjectInternal(Subject* subject)
    {
        m_subjects.erase(std::remove(m_subjects.begin(), m_subjects.end(), subject), m_subjects.end());
    }
}
