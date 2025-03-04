/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Acts as a critical section which processes can use to block each other.

    @see CriticalSection

    @tags{Core}
*/
class JUCE_API  InterProcessLock
{
public:
    //==============================================================================
    /** Creates a lock object.
        @param name   a name that processes will use to identify this lock object
    */
    explicit InterProcessLock (const String& name);

    /** Destructor.
        This will also release the lock if it's currently held by this process.
    */
    ~InterProcessLock();

    //==============================================================================
    /** Attempts to lock the critical section.

        @param timeOutMillisecs  how many milliseconds to wait if the lock is already
                                 held by another process - a value of 0 will return
                                 immediately, negative values will wait forever
        @returns    true if the lock could be gained within the timeout period, or
                    false if the timeout expired.
    */
    bool enter (int timeOutMillisecs = -1);

    /** Releases the lock if it's currently held by this process. */
    void exit();

    //==============================================================================
    /**
        Automatically locks and unlocks an InterProcessLock object.

        This works like a ScopedLock, but using an InterprocessLock rather than
        a CriticalSection.

        @see ScopedLock
    */
    class ScopedLockType
    {
    public:
        //==============================================================================
        /** Creates a scoped lock.

            As soon as it is created, this will lock the InterProcessLock, and
            when the ScopedLockType object is deleted, the InterProcessLock will
            be unlocked.

            Note that since an InterprocessLock can fail due to errors, you should check
            isLocked() to make sure that the lock was successful before using it.

            Make sure this object is created and deleted by the same thread,
            otherwise there are no guarantees what will happen! Best just to use it
            as a local stack object, rather than creating one with the new() operator.
        */
        explicit ScopedLockType (InterProcessLock& l)        : ipLock (l) { lockWasSuccessful = l.enter(); }

        /** Destructor.

            The InterProcessLock will be unlocked when the destructor is called.

            Make sure this object is created and deleted by the same thread,
            otherwise there are no guarantees what will happen!
        */
        inline ~ScopedLockType()                             { ipLock.exit(); }

        /** Returns true if the InterProcessLock was successfully locked. */
        bool isLocked() const noexcept                       { return lockWasSuccessful; }

    private:
        //==============================================================================
        InterProcessLock& ipLock;
        bool lockWasSuccessful;

        JUCE_DECLARE_NON_COPYABLE (ScopedLockType)
    };

private:
    //==============================================================================
    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    CriticalSection lock;
    String name;

    JUCE_DECLARE_NON_COPYABLE (InterProcessLock)
};

} // namespace juce
