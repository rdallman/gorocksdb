package gorocksdb

import (
	"os"
	"testing"
	. "github.com/smartystreets/goconvey/convey"
)

type testMergeOperator struct {
	initiated bool
}

func (self *testMergeOperator) FullMerge(key, existingValue []byte, operands [][]byte) ([]byte, bool) {
	for _, operand := range operands {
		existingValue = append(existingValue, operand...)
	}

	return existingValue, true
}

func (self *testMergeOperator) PartialMerge(key, leftOperand, rightOperand []byte) ([]byte, bool) {
	return append(leftOperand, rightOperand...), true
}

func (self *testMergeOperator) Name() string {
	self.initiated = true
	return "gorocksdb.test"
}

func TestNewMergeOperator(t *testing.T) {
	dbName := os.TempDir() + "/TestNewMergeOperator"

	Convey("Subject: Custom merge operator", t, func() {
		Convey("When passed to the db as merge operator then it should not panic", func() {
			merger := &testMergeOperator{}
			options := NewDefaultOptions()
			DestroyDb(dbName, options)
			options.SetCreateIfMissing(true)
			options.SetMergeOperator(merger)
			//options.SetMaxSuccessiveMerges(5)

			db, err := OpenDb(options, dbName)
			So(err, ShouldBeNil)
			So(merger.initiated, ShouldBeTrue)

			wo := NewDefaultWriteOptions()
			So(db.Put(wo, []byte("foo"), []byte("foo")), ShouldBeNil)
			So(db.Merge(wo, []byte("foo"), []byte("bar")), ShouldBeNil)

			value, err := db.Get(NewDefaultReadOptions(), []byte("foo"))
			So(err, ShouldBeNil)
			So(value.Data(), ShouldResemble, []byte("foobar"))
			value.Free()

			db.Close()
			DestroyDb(dbName, options)

			dbopts := NewDefaultOptions()
			dbopts.SetCreateIfMissing(true)
			dbopts.SetCreateMissingColumnFamilies(true)

			cfopts := NewDefaultOptions()
			cfopts.SetMergeOperator(new(testMergeOperator))
			defcfd := ColumnFamilyDescriptor{"default", NewDefaultOptions()}
			cf1 := ColumnFamilyDescriptor{"hi", cfopts}
			cfds := []ColumnFamilyDescriptor{defcfd, cf1}
			db, cfs, err := OpenDbWithColumnFamilies(dbopts, dbName, cfds)
			So(err, ShouldBeNil)
			So(merger.initiated, ShouldBeTrue)

			wb := NewWriteBatch()
			wb.PutCF(cfs[1], []byte("foo"), []byte("foo"))
			wb.MergeCF(cfs[1], []byte("foo"), []byte("bar"))
			So(db.Write(wo, wb), ShouldBeNil)

			value, err = db.GetCF(NewDefaultReadOptions(), cfs[1], []byte("foo"))
			So(err, ShouldBeNil)
			So(value.Data(), ShouldResemble, []byte("foobar"))
			value.Free()
		})
	})
}

func TestMergeOperatorNonExisitingValue(t *testing.T) {
	dbName := os.TempDir() + "/TestMergeOperatorNonExisitingValue"

	Convey("Subject: Merge of a non-existing value", t, func() {
		merger := &testMergeOperator{}

		options := NewDefaultOptions()
		DestroyDb(dbName, options)
		options.SetCreateIfMissing(true)
		options.SetMergeOperator(merger)
		options.SetMaxSuccessiveMerges(5)

		db, err := OpenDb(options, dbName)
		So(err, ShouldBeNil)
		So(merger.initiated, ShouldBeTrue)

		Convey("When merge a non-existing value with 'bar' then the new value should be 'bar'", func() {
			wo := NewDefaultWriteOptions()
			So(db.Merge(wo, []byte("notexists"), []byte("bar")), ShouldBeNil)

			Convey("Then the new value should be 'bar'", func() {
				value, err := db.Get(NewDefaultReadOptions(), []byte("notexists"))
				So(err, ShouldBeNil)
				So(value.Data(), ShouldResemble, []byte("bar"))
				value.Free()
			})
		})
	})
}
