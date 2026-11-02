# Portage des tests de la v1

Chacun des 329 tests de la v1 et le test de la v2 qui le remplace. `checks/test_portage.py` vérifie cette table.

- **239** tests gardent leur nom.
- Tous les tests vérifient maintenant les fuites et les accès hors limites, pas seulement les 22 tests `valgrind/*`.
- Les 21 tests sur NULL et `atoi/overflow_long` passent dans la section `info` : le sujet ne définit pas ces cas, ils sont affichés sans compter dans le score.

| v1 | v2 | Note |
|---|---|---|
| `atoi/all_whitespace` | `atoi/all_whitespace` |  |
| `atoi/basic_negative` | `atoi/basic_negative` |  |
| `atoi/basic_positive` | `atoi/basic_positive` |  |
| `atoi/double_signs` | `atoi/double_signs` |  |
| `atoi/empty_string` | `atoi/empty_string` |  |
| `atoi/large_number` | `atoi/large_number` |  |
| `atoi/leading_whitespace` | `atoi/leading_whitespace` |  |
| `atoi/leading_zeros` | `atoi/leading_zeros` |  |
| `atoi/minus_plus` | `atoi/minus_plus` |  |
| `atoi/multiple_signs` | `atoi/multiple_signs` |  |
| `atoi/negative_large` | `atoi/negative_large` |  |
| `atoi/no_digits` | `atoi/no_digits` |  |
| `atoi/only_whitespace` | `atoi/only_whitespace` |  |
| `atoi/overflow_long` | `info/atoi_overflow_positive` | indéfini selon le man, ne doit pas imposer le résultat de la glibc |
| `atoi/plus_newline` | `atoi/plus_newline` |  |
| `atoi/plus_plus` | `atoi/plus_plus` |  |
| `atoi/plus_space` | `atoi/plus_space` |  |
| `atoi/semicolon_separator` | `atoi/semicolon_separator` |  |
| `atoi/slash_separator` | `atoi/slash_separator` |  |
| `atoi/space_between_sign` | `atoi/space_between_sign` |  |
| `atoi/trailing_chars` | `atoi/trailing_chars` |  |
| `atoi/with_plus` | `atoi/with_plus` |  |
| `atoi/zero` | `atoi/zero` |  |
| `bzero/basic` | `bzero/basic` |  |
| `bzero/large` | `bzero/large` |  |
| `bzero/partial` | `bzero/partial` |  |
| `bzero/single_byte` | `bzero/single_byte` |  |
| `bzero/size_zero` | `bzero/size_zero` |  |
| `bzero/struct` | `bzero/struct` |  |
| `calloc/basic` | `calloc/basic` |  |
| `calloc/char_array` | `calloc/char_array` |  |
| `calloc/large_allocation` | `calloc/large_allocation` |  |
| `calloc/single_element` | `calloc/single_element` |  |
| `calloc/struct_allocation` | `calloc/struct_allocation` |  |
| `calloc/write_and_read` | `calloc/write_and_read` |  |
| `calloc/zero_count` | `calloc/zero_count` |  |
| `calloc/zero_size` | `calloc/zero_size` |  |
| `caseconv/all_letters` | `toupper/all_letters` | et tolower/all_letters |
| `caseconv/edge_cases` | `toupper/boundaries` | et tolower/boundaries |
| `caseconv/extended_ascii` | `toupper/limits` | et tolower/limits |
| `caseconv/numbers_and_special` | `toupper/numbers_and_special` | et tolower/numbers_and_special |
| `caseconv/tolower_basic` | `tolower/basic` |  |
| `caseconv/tolower_non_alpha` | `tolower/non_alpha` |  |
| `caseconv/toupper_basic` | `toupper/basic` |  |
| `caseconv/toupper_non_alpha` | `toupper/non_alpha` |  |
| `charclass/comprehensive` | `isalpha/mixed_string` | retour exact 1 ou 0 |
| `charclass/isalnum_basic` | `isalnum/basic` | retour exact 1 ou 0 |
| `charclass/isalpha_basic` | `isalpha/basic` | retour exact 1 ou 0 |
| `charclass/isalpha_edge` | `isalpha/edge` | retour exact 1 ou 0 |
| `charclass/isascii_basic` | `isascii/basic` | retour exact 1 ou 0 |
| `charclass/isdigit_basic` | `isdigit/basic` | retour exact 1 ou 0 |
| `charclass/isprint_basic` | `isprint/basic` | retour exact 1 ou 0 |
| `itoa/int_max` | `itoa/int_max` |  |
| `itoa/int_min` | `itoa/int_min` |  |
| `itoa/large_negative` | `itoa/large_negative` |  |
| `itoa/large_positive` | `itoa/large_positive` |  |
| `itoa/negative` | `itoa/negative` |  |
| `itoa/positive` | `itoa/positive` |  |
| `itoa/powers_of_ten` | `itoa/powers_of_ten` |  |
| `itoa/single_digit_negative` | `itoa/single_digit_negative` |  |
| `itoa/single_digit_positive` | `itoa/single_digit_positive` |  |
| `itoa/zero` | `itoa/zero` |  |
| `list/complex_operations` | `lstclear/complex_operations` |  |
| `list/lstadd_back_basic` | `lstadd_back/basic` |  |
| `list/lstadd_back_empty` | `lstadd_back/empty` |  |
| `list/lstadd_front_basic` | `lstadd_front/basic` |  |
| `list/lstadd_front_empty` | `lstadd_front/empty` |  |
| `list/lstclear_basic` | `lstclear/basic` | vérifie aussi del et les fuites |
| `list/lstdelone_basic` | `lstdelone/basic` | vérifie aussi del et les fuites |
| `list/lstiter_basic` | `lstiter/basic` |  |
| `list/lstiter_empty` | `lstiter/empty` |  |
| `list/lstlast_basic` | `lstlast/basic` |  |
| `list/lstlast_null` | `lstlast/null` |  |
| `list/lstlast_single` | `lstlast/single` |  |
| `list/lstmap_basic` | `lstmap/basic` |  |
| `list/lstmap_empty` | `lstmap/empty` |  |
| `list/lstnew_basic` | `lstnew/basic` |  |
| `list/lstnew_null` | `lstnew/null` |  |
| `list/lstsize_basic` | `lstsize/basic` |  |
| `list/lstsize_empty` | `lstsize/empty` |  |
| `list/lstsize_single` | `lstsize/single` |  |
| `memchr/basic_found` | `memchr/basic_found` |  |
| `memchr/binary_data` | `memchr/binary_data` |  |
| `memchr/first_byte` | `memchr/first_byte` |  |
| `memchr/last_byte` | `memchr/last_byte` |  |
| `memchr/negative_char` | `memchr/negative_char` |  |
| `memchr/not_found` | `memchr/not_found` |  |
| `memchr/partial_search` | `memchr/partial_search` |  |
| `memchr/struct_data` | `memchr/struct_data` |  |
| `memchr/zero_byte` | `memchr/zero_byte` |  |
| `memchr/zero_length` | `memchr/zero_length` |  |
| `memcmp/basic_different` | `memcmp/basic_different` |  |
| `memcmp/basic_equal` | `memcmp/basic_equal` |  |
| `memcmp/binary_data` | `memcmp/binary_data` |  |
| `memcmp/continues_after_null` | `memcmp/continues_after_null` |  |
| `memcmp/large_data` | `memcmp/large_data` |  |
| `memcmp/negative_bytes` | `memcmp/negative_bytes` |  |
| `memcmp/null_bytes_beyond_string` | `memcmp/null_bytes_beyond_string` |  |
| `memcmp/partial_equal` | `memcmp/partial_equal` |  |
| `memcmp/signed_char_edge` | `memcmp/signed_char_edge` |  |
| `memcmp/struct_data` | `memcmp/struct_data` |  |
| `memcmp/with_null_bytes` | `memcmp/with_null_bytes` |  |
| `memcmp/zero_length` | `memcmp/zero_length` |  |
| `memcpy/alignment_test` | `memcpy/alignment` |  |
| `memcpy/basic` | `memcpy/basic` |  |
| `memcpy/binary_data` | `memcpy/binary_data` |  |
| `memcpy/large` | `memcpy/large` |  |
| `memcpy/partial` | `memcpy/partial` |  |
| `memcpy/return_value` | `memcpy/return_value` |  |
| `memcpy/struct` | `memcpy/struct` |  |
| `memcpy/zero_size` | `memcpy/zero_size` |  |
| `memmove/basic` | `memmove/basic` |  |
| `memmove/binary_overlap` | `memmove/binary_overlap` |  |
| `memmove/exact_overlap` | `memmove/same_pointer` | doublon dans la v1 |
| `memmove/large_overlap` | `memmove/large_overlap` |  |
| `memmove/no_overlap` | `memmove/no_overlap` |  |
| `memmove/overlap_backward` | `memmove/overlap_backward` |  |
| `memmove/overlap_forward` | `memmove/overlap_forward` |  |
| `memmove/same_pointer` | `memmove/same_pointer` |  |
| `memmove/single_byte_overlap` | `memmove/single_byte_overlap` |  |
| `memmove/zero_size` | `memmove/zero_size` |  |
| `memset/basic` | `memset/basic` |  |
| `memset/large` | `memset/large` |  |
| `memset/negative_char` | `memset/negative_char` |  |
| `memset/partial` | `memset/partial` |  |
| `memset/return_value` | `memset/return_value` |  |
| `memset/size_zero` | `memset/size_zero` |  |
| `memset/zero` | `memset/zero` |  |
| `overprotection/atoi_null_should_crash` | `info/atoi_null` | non compté : NULL est indéfini |
| `overprotection/memcpy_null_should_crash` | `info/memcpy_null_dest` | non compté : NULL est indéfini |
| `overprotection/memmove_null_should_crash` | `info/memmove_null_dest` | non compté : NULL est indéfini |
| `overprotection/memset_null_should_crash` | `info/memset_null` | non compté : NULL est indéfini |
| `overprotection/strchr_null_should_crash` | `info/strchr_null` | non compté : NULL est indéfini |
| `overprotection/strlen_null_should_crash` | `info/strlen_null` | non compté : NULL est indéfini |
| `overprotection/strncmp_null_should_crash` | `info/strncmp_null` | non compté : NULL est indéfini |
| `overprotection/strrchr_null_should_crash` | `info/strrchr_null` | non compté : NULL est indéfini |
| `putfd/int_min_max` | `putnbr_fd/int_min_max_together` | vérifie le contenu exact, plus seulement la taille |
| `putfd/putchar_fd_basic` | `putchar_fd/basic` | pas de fichier dans /tmp ni de usleep |
| `putfd/putendl_fd_basic` | `putendl_fd/basic` |  |
| `putfd/putnbr_fd_negative` | `putnbr_fd/negative` |  |
| `putfd/putnbr_fd_positive` | `putnbr_fd/positive` |  |
| `putfd/putnbr_fd_zero` | `putnbr_fd/zero` |  |
| `putfd/putstr_fd_basic` | `putstr_fd/basic` |  |
| `putfd/putstr_fd_empty` | `putstr_fd/empty` |  |
| `putfd/putstr_fd_null` | `info/putstr_fd_null` | non compté : NULL est indéfini |
| `putfd/stdout_test` | `putstr_fd/sequence_of_all_put_functions` | vérifie le contenu, l'ancien test ne vérifiait rien |
| `split/basic` | `split/basic` |  |
| `split/complex_example` | `split/complex_example` |  |
| `split/consecutive_delimiters_end` | `split/consecutive_delimiters_end` |  |
| `split/consecutive_delimiters_start` | `split/consecutive_delimiters_start` |  |
| `split/delimiter_not_found` | `split/no_delimiter_found` | doublon dans la v1 |
| `split/different_delimiter` | `split/different_delimiter` |  |
| `split/empty_string` | `split/empty_string` |  |
| `split/leading_trailing_delim` | `split/leading_trailing_delim` |  |
| `split/mixed_delimiters` | `split/mixed_delimiters` |  |
| `split/multiple_delimiters` | `split/multiple_delimiters` |  |
| `split/no_delimiter_found` | `split/no_delimiter_found` |  |
| `split/null_input` | `info/split_null` | non compté, doublon de split/null_string dans la v1 |
| `split/null_string` | `info/split_null` | non compté : NULL est indéfini |
| `split/only_delimiters` | `split/only_delimiters` |  |
| `split/single_char` | `split/single_char` |  |
| `split/single_char_words` | `split/single_char_words` |  |
| `split/single_word` | `split/single_word` |  |
| `split/special_chars` | `split/special_chars` |  |
| `strchr/basic` | `strchr/basic` |  |
| `strchr/empty_string` | `strchr/empty_string` |  |
| `strchr/first_char` | `strchr/first_char` |  |
| `strchr/last_char` | `strchr/last_char` |  |
| `strchr/multiple_occurrences` | `strchr/multiple_occurrences` |  |
| `strchr/not_found` | `strchr/not_found` |  |
| `strchr/null_terminator` | `strchr/null_terminator` |  |
| `strchr/special_chars` | `strchr/special_chars` |  |
| `strchr/unsigned_char_comparison` | `strchr/unsigned_char_comparison` |  |
| `strdup/basic` | `strdup/basic` |  |
| `strdup/empty_string` | `strdup/empty_string` |  |
| `strdup/independent_copy` | `strdup/independent_copy` |  |
| `strdup/long_string` | `strdup/long_string` |  |
| `strdup/null_terminator` | `strdup/null_terminator` |  |
| `strdup/numbers_and_symbols` | `strdup/numbers_and_symbols` |  |
| `strdup/single_char` | `strdup/single_char` |  |
| `strdup/special_chars` | `strdup/special_chars` |  |
| `striteri/alternating_case` | `striteri/alternating_case` |  |
| `striteri/basic` | `striteri/basic` |  |
| `striteri/empty_string` | `striteri/empty_string` |  |
| `striteri/long_string` | `striteri/long_string` |  |
| `striteri/null_function` | `info/striteri_null_function` | non compté : NULL est indéfini |
| `striteri/null_string` | `info/striteri_null_string` | non compté : NULL est indéfini |
| `striteri/preserve_non_alpha` | `striteri/preserve_non_alpha` |  |
| `striteri/set_to_index` | `striteri/set_to_index` |  |
| `striteri/single_char` | `striteri/single_char` |  |
| `striteri/toupper_func` | `striteri/toupper_func` |  |
| `striteri/with_index` | `striteri/with_index` |  |
| `strjoin/basic` | `strjoin/basic` |  |
| `strjoin/both_empty` | `strjoin/both_empty` |  |
| `strjoin/both_null` | `info/strjoin_both_null` | non compté : NULL est indéfini |
| `strjoin/empty_first` | `strjoin/empty_first` |  |
| `strjoin/empty_second` | `strjoin/empty_second` |  |
| `strjoin/long_strings` | `strjoin/long_strings` |  |
| `strjoin/null_first` | `info/strjoin_null_first` | non compté : NULL est indéfini |
| `strjoin/null_second` | `info/strjoin_null_second` | non compté : NULL est indéfini |
| `strjoin/single_chars` | `strjoin/single_chars` |  |
| `strjoin/special_chars` | `strjoin/special_chars` |  |
| `strlcat/basic` | `strlcat/basic` |  |
| `strlcat/empty_dest` | `strlcat/empty_dest` |  |
| `strlcat/empty_src` | `strlcat/empty_src` |  |
| `strlcat/multiple_concat` | `strlcat/multiple_concat` |  |
| `strlcat/size_equal_dest` | `strlcat/size_equal_dest` |  |
| `strlcat/size_less_than_dest` | `strlcat/size_less_than_dest` |  |
| `strlcat/size_zero` | `strlcat/size_zero` |  |
| `strlcat/truncate` | `strlcat/truncate` |  |
| `strlcpy/basic` | `strlcpy/basic` |  |
| `strlcpy/empty_src` | `strlcpy/empty_src` |  |
| `strlcpy/exact_size` | `strlcpy/exact_size` |  |
| `strlcpy/long_string` | `strlcpy/long_string` |  |
| `strlcpy/size_one` | `strlcpy/size_one` |  |
| `strlcpy/size_zero` | `strlcpy/size_zero` |  |
| `strlcpy/special_chars` | `strlcpy/special_chars` |  |
| `strlcpy/truncate` | `strlcpy/truncate` |  |
| `strlen/basic` | `strlen/basic` |  |
| `strlen/empty` | `strlen/empty` |  |
| `strlen/long_string` | `strlen/long_string` |  |
| `strlen/single_char` | `strlen/single_char` |  |
| `strlen/spaces` | `strlen/spaces` |  |
| `strlen/special_chars` | `strlen/special_chars` |  |
| `strlen/with_numbers` | `strlen/with_numbers` |  |
| `strmapi/alternating_case` | `strmapi/alternating_case` |  |
| `strmapi/basic` | `strmapi/basic` |  |
| `strmapi/empty_string` | `strmapi/empty_string` |  |
| `strmapi/null_function` | `info/strmapi_null_function` | non compté : NULL est indéfini |
| `strmapi/null_string` | `info/strmapi_null_string` | non compté : NULL est indéfini |
| `strmapi/numbers_to_letters` | `strmapi/numbers_to_letters` |  |
| `strmapi/preserve_non_alpha` | `strmapi/preserve_non_alpha` |  |
| `strmapi/single_char` | `strmapi/single_char` |  |
| `strmapi/toupper_func` | `strmapi/toupper_func` |  |
| `strmapi/with_index` | `strmapi/with_index` |  |
| `strncmp/basic_different` | `strncmp/basic_different` |  |
| `strncmp/basic_equal` | `strncmp/basic_equal` |  |
| `strncmp/case_sensitive` | `strncmp/case_sensitive` |  |
| `strncmp/different_lengths` | `strncmp/different_lengths` |  |
| `strncmp/empty_strings` | `strncmp/empty_strings` |  |
| `strncmp/one_empty` | `strncmp/one_empty` |  |
| `strncmp/partial_compare` | `strncmp/partial_compare` |  |
| `strncmp/unsigned_char` | `strncmp/unsigned_char` |  |
| `strncmp/zero_length` | `strncmp/zero_length` |  |
| `strnstr/at_beginning` | `strnstr/at_beginning` |  |
| `strnstr/at_end` | `strnstr/at_end` |  |
| `strnstr/basic_found` | `strnstr/basic_found` |  |
| `strnstr/basic_not_found` | `strnstr/basic_not_found` |  |
| `strnstr/empty_haystack_with_needle` | `strnstr/empty_haystack_with_needle` |  |
| `strnstr/empty_needle` | `strnstr/empty_needle` |  |
| `strnstr/empty_needle_empty_haystack` | `strnstr/empty_needle_empty_haystack` |  |
| `strnstr/exact_match` | `strnstr/exact_match` |  |
| `strnstr/haystack_empty_needle` | `strnstr/haystack_empty_needle` |  |
| `strnstr/multiple_occurrences` | `strnstr/multiple_occurrences` |  |
| `strnstr/needle_longer_than_haystack` | `strnstr/needle_longer_than_haystack` |  |
| `strnstr/needle_longer_than_len` | `strnstr/needle_longer_than_len` |  |
| `strnstr/overlapping_pattern` | `strnstr/overlapping_pattern` |  |
| `strnstr/partial_len` | `strnstr/partial_len` |  |
| `strnstr/partial_match_boundary` | `strnstr/partial_match_boundary` |  |
| `strnstr/signed_chars` | `strnstr/signed_chars` |  |
| `strnstr/single_char` | `strnstr/single_char` |  |
| `strnstr/zero_len` | `strnstr/zero_len` |  |
| `strrchr/basic` | `strrchr/basic` |  |
| `strrchr/empty_string` | `strrchr/empty_string` |  |
| `strrchr/first_char` | `strrchr/first_char` |  |
| `strrchr/last_char` | `strrchr/last_char` |  |
| `strrchr/multiple_occurrences` | `strrchr/multiple_occurrences` |  |
| `strrchr/not_found` | `strrchr/not_found` |  |
| `strrchr/null_terminator` | `strrchr/null_terminator` |  |
| `strrchr/repeated_char` | `strrchr/repeated_char` |  |
| `strrchr/same_as_strchr` | `strrchr/same_as_strchr` |  |
| `strtrim/all_trimmed` | `strtrim/all_trimmed` |  |
| `strtrim/basic` | `strtrim/basic` |  |
| `strtrim/complex_set` | `strtrim/complex_set` |  |
| `strtrim/empty_both` | `strtrim/empty_both` |  |
| `strtrim/empty_set` | `strtrim/empty_set` |  |
| `strtrim/empty_set_nochange` | `strtrim/empty_set_nochange` |  |
| `strtrim/empty_string` | `strtrim/empty_string` |  |
| `strtrim/empty_string_with_set` | `strtrim/empty_string_with_set` |  |
| `strtrim/multiple_chars` | `strtrim/multiple_chars` |  |
| `strtrim/no_match_set` | `strtrim/no_match_set` |  |
| `strtrim/no_trim_needed` | `strtrim/no_trim_needed` |  |
| `strtrim/null_set` | `info/strtrim_null_set` | non compté : NULL est indéfini |
| `strtrim/null_string` | `info/strtrim_null_string` | non compté : NULL est indéfini |
| `strtrim/only_beginning` | `strtrim/only_beginning` |  |
| `strtrim/only_end` | `strtrim/only_end` |  |
| `strtrim/prefix_only` | `strtrim/prefix_only` |  |
| `strtrim/punctuation` | `strtrim/punctuation` |  |
| `strtrim/single_char` | `strtrim/single_char` |  |
| `strtrim/suffix_only` | `strtrim/suffix_only` |  |
| `strtrim/whitespace_chars` | `strtrim/whitespace_chars` |  |
| `substr/at_end` | `substr/at_end` |  |
| `substr/basic` | `substr/basic` |  |
| `substr/empty_source` | `substr/empty_source` |  |
| `substr/empty_string_with_len` | `substr/empty_string_with_len` |  |
| `substr/empty_string_zero_len` | `substr/empty_string_zero_len` |  |
| `substr/exact_boundaries` | `substr/exact_boundaries` |  |
| `substr/full_string` | `substr/full_string` |  |
| `substr/large_start_max_len` | `substr/large_start_max_len` |  |
| `substr/len_beyond_string` | `substr/len_beyond_string` |  |
| `substr/max_len` | `substr/max_len` |  |
| `substr/middle_section` | `substr/middle_section` |  |
| `substr/null_source` | `info/substr_null` | non compté : NULL est indéfini |
| `substr/single_char` | `substr/single_char` |  |
| `substr/start_beyond_empty` | `substr/start_beyond_empty` |  |
| `substr/start_beyond_string` | `substr/start_beyond_string` |  |
| `substr/zero_length` | `substr/zero_length` |  |
| `valgrind/calloc_basic` | `calloc/basic` | fuites vérifiées partout |
| `valgrind/calloc_zero_count` | `calloc/zero_count` | fuites vérifiées partout |
| `valgrind/calloc_zero_size` | `calloc/zero_size` | fuites vérifiées partout |
| `valgrind/itoa_negative` | `itoa/negative` | fuites vérifiées partout |
| `valgrind/itoa_positive` | `itoa/positive` | fuites vérifiées partout |
| `valgrind/itoa_zero` | `itoa/zero` | fuites vérifiées partout |
| `valgrind/lstclear_basic` | `lstclear/frees_the_contents_through_del` | fuites vérifiées partout |
| `valgrind/lstmap_basic` | `lstmap/basic` | fuites vérifiées partout |
| `valgrind/lstnew_basic` | `lstnew/basic` | fuites vérifiées partout |
| `valgrind/split_basic` | `split/different_delimiter` | fuites vérifiées partout |
| `valgrind/split_empty_string` | `split/empty_string` | fuites vérifiées partout |
| `valgrind/split_no_delimiter` | `split/no_delimiter_found` | fuites vérifiées partout |
| `valgrind/strdup_basic` | `strdup/basic` | fuites vérifiées partout |
| `valgrind/strdup_empty` | `strdup/empty_string` | fuites vérifiées partout |
| `valgrind/strjoin_basic` | `strjoin/basic` | fuites vérifiées partout |
| `valgrind/strjoin_empty_strings` | `strjoin/both_empty` | fuites vérifiées partout |
| `valgrind/strmapi_basic` | `strmapi/basic` | fuites vérifiées partout |
| `valgrind/strmapi_empty` | `strmapi/empty_string` | fuites vérifiées partout |
| `valgrind/strtrim_all_trimmed` | `strtrim/all_trimmed` | fuites vérifiées partout |
| `valgrind/strtrim_basic` | `strtrim/basic` | fuites vérifiées partout |
| `valgrind/substr_basic` | `substr/basic` | fuites vérifiées partout |
| `valgrind/substr_beyond_string` | `substr/start_beyond_string` | fuites vérifiées partout |
