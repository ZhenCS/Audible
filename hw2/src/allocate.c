
/*
 * Allocate storage for the various data structures
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "global.h"
#include "gradedb.h"
#include "stats.h"
#include "allocate.h"

char *memerr = "Unable to allocate memory.";

Professor *newprofessor()
{
        Professor *p;
        if((p = (Professor *)malloc(sizeof(Professor))) == NULL)
                fatal(memerr, 0);
        return(p);
}

Assistant *newassistant()
{
        Assistant *a;
        if((a = (Assistant *)malloc(sizeof(Assistant))) == NULL)
                fatal(memerr, 0);
        return(a);
}

Student *newstudent()
{
        Student *s;
        if((s = (Student *)malloc(sizeof(Student))) == NULL)
                fatal(memerr, 0);
        return(s);
}

Section *newsection()
{
        Section *s;
        if((s = (Section *)malloc(sizeof(Section))) == NULL)
                fatal(memerr, 0);
        return(s);
}

Assignment *newassignment()
{
        Assignment *a;
        if((a = (Assignment *)malloc(sizeof(Assignment))) == NULL)
                fatal(memerr, 0);
        return(a);
}

Course *newcourse()
{
        Course *c;
        if((c = (Course *)malloc(sizeof(Course))) == NULL)
                fatal(memerr, 0);
        return(c);
}

Score *newscore()
{
        Score *s;
        if((s = (Score *)malloc(sizeof(Score))) == NULL)
                fatal(memerr, 0);
        return(s);
}

char *newstring(tp, size)
char *tp;
int size;
{
        char *s, *cp;
        if((s = (char *)malloc(size)) == NULL)
                fatal(memerr, 0);
        cp = s;
        while(size-- > 0) *cp++ = *tp++;
        return(s);
}

Freqs *newfreqs()
{
        Freqs *f;
        if((f = (Freqs *)malloc(sizeof(Freqs))) == NULL)
                fatal(memerr, 0);
        return(f);
}

Classstats *newclassstats()
{
        Classstats *c;
        if((c = (Classstats *)malloc(sizeof(Classstats))) == NULL)
                fatal(memerr, 0);
        return(c);

}

Sectionstats *newsectionstats()
{
        Sectionstats *s;
        if((s = (Sectionstats *)malloc(sizeof(Sectionstats))) == NULL)
                fatal(memerr, 0);
        return(s);

}

Stats *newstats()
{
        Stats *s;
        if((s = (Stats *)malloc(sizeof(Stats))) == NULL)
                fatal(memerr, 0);
        return(s);
}

Ifile *newifile()
{
        Ifile *f;
        if((f = (Ifile *)malloc(sizeof(Ifile))) == NULL)
                fatal(memerr, 0);
        return(f);
}
