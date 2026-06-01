#ifndef DOUBLY_LINKED_LIST_H_
#define DOUBLY_LINKED_LIST_H_

#include <stddef.h>
#include <stdint.h>

typedef struct Link
{
    struct Link *pxPrev;
    struct Link *pxNext;
} Link_t;

/**
 * @brief Initializes the given list head to an empty list.
 *
 * @param[in] pxHead The given list head to initialize.
 */
#define listINIT_HEAD(pxHead)        \
    {                                \
        (pxHead)->pxPrev = (pxHead); \
        (pxHead)->pxNext = (pxHead); \
    }

/**
 * @brief Adds the given new node to the given list.
 *
 * @param[in] pxHead The head of the given list.
 * @param[in] pxLink The given new node to be added to the given
 * list.
 */
#define listADD(pxHead, pxLink)                  \
    {                                            \
        Link_t *pxPrevLink = (pxHead);           \
        Link_t *pxNextLink = ((pxHead)->pxNext); \
                                                \
        (pxLink)->pxNext = pxNextLink;           \
        pxNextLink->pxPrev = (pxLink);           \
        pxPrevLink->pxNext = (pxLink);           \
        (pxLink)->pxPrev = (pxPrevLink);         \
    }

/**
 * @brief Removes the given node from the list it is part of.
 *
 * If the given node is not a part of any list (i.e. next and previous
 * nodes are NULL), nothing happens.
 *
 * @param[in] pxLink The given node to remove from the list.
 */
#define listREMOVE(pxLink)                                         \
    {                                                              \
        if (( pxLink)->pxNext != NULL && (pxLink)->pxPrev != NULL) \
        {                                                          \
            (pxLink)->pxPrev->pxNext = (pxLink)->pxNext;           \
            (pxLink)->pxNext->pxPrev = (pxLink)->pxPrev;           \
        }                                                          \
        (pxLink)->pxPrev = NULL;                                   \
        (pxLink)->pxNext = NULL;                                   \
    }

/**
 * @brief Given the head of a list, checks if the list is empty.
 *
 * @param[in] pxHead The head of the given list.
 */
#define listIS_EMPTY(pxHead)    (((pxHead) == NULL) || ((pxHead)->pxNext == (pxHead)))

/**
 * @brief Removes the first node from the given list and returns it.
 *
 * Removes the first node from the given list and assigns it to the
 * pxLink parameter. If the list is empty, it assigns NULL to the
 * pxLink.
 *
 * @param[in] pxHead The head of the list from which to remove the
 * first node.
 * @param[out] pxLink The output parameter to receive the removed
 * node.
 */
#define listPOP(pxHead, pxLink)                                         \
    {                                                                   \
        if (listIS_EMPTY((pxHead))) {                                   \
            (pxLink) = NULL;                                            \
        } else {                                                        \
            (pxLink) = (pxHead)->pxNext;                                \
            if ((pxLink)->pxNext != NULL && (pxLink)->pxPrev != NULL) { \
                (pxLink)->pxPrev->pxNext = (pxLink)->pxNext;            \
                (pxLink)->pxNext->pxPrev = (pxLink)->pxPrev;            \
            }                                                           \
            (pxLink)->pxPrev = NULL;                                    \
            (pxLink)->pxNext = NULL;                                    \
        }                                                               \
    }

/**
 * @brief Merges a list into a given list.
 *
 * @param[in] pxHeadResultList The head of the given list into which the
 * other list should be merged.
 * @param[in] pxHeadListToMerge The head of the list to be merged into the
 * given list.
 */
#define listMERGE(pxHeadResultList, pxHeadListToMerge)                         \
    {                                                                          \
        if (!listIS_EMPTY((pxHeadListToMerge))) {                              \
            (pxHeadListToMerge)->pxPrev->pxNext = (pxHeadResultList)->pxNext;  \
            (pxHeadResultList)->pxNext->pxPrev = (pxHeadListToMerge)->pxPrev;  \
            (pxHeadListToMerge)->pxNext->pxPrev = (pxHeadResultList);          \
            (pxHeadResultList)->pxNext = (pxHeadListToMerge)->pxNext;          \
            listINIT_HEAD((pxHeadListToMerge));                                \
        }                                                                      \
    }

/**
 * @brief Helper macro to iterate over a list. pxLink contains the link node
 * in each iteration.
 */
#define listFOR_EACH(pxLink, pxHead)  \
    for ((pxLink) = (pxHead)->pxNext; \
        (pxLink) != (pxHead);         \
        (pxLink) = (pxLink)->pxNext)

/**
 * @brief Helper macro to iterate over a list. It is safe to destroy/free the
 * nodes while iterating. pxLink contains the link node in each iteration.
 */
#define listFOR_EACH_SAFE(pxLink, pxTempLink, pxHead)                  \
    for ((pxLink) = (pxHead)->pxNext, (pxTempLink) = (pxLink)->pxNext; \
        (pxLink) != (pxHead);                                          \
        (pxLink) = (pxTempLink), (pxTempLink) = (pxLink)->pxNext )

/**
 * @brief Given the pointer to the link member (of type Link_t) in a struct,
 * extracts the pointer to the containing struct.
 *
 * @param[in] pxLink The pointer to the link member.
 * @param[in] type The type of the containing struct.
 * @param[in] member Name of the link member in the containing struct.
 */
#define listCONTAINER(pxLink, type, member)    ((type *)((uint8_t *)(pxLink) - (uint8_t *) (&((type*) 0)->member)))

#endif
